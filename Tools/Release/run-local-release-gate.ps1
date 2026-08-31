param(
    [Parameter(Mandatory = $true)]
    [string[]]$EngineRoots,

    [switch]$IncludeLinux,

    [string]$LinuxToolchainBase = "C:\UnrealToolchains",

    [string]$LinuxPackageBase = "",

    [switch]$AllowDirty,

    [ValidateRange(1, 128)]
    [int]$WindowsMaxParallelActions = 8
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if ($EngineRoots.Count -ne 3) {
    throw "Provide the UE 5.6, 5.7, and 5.8 engine roots in that order."
}

$ExpectedVersions = @("UE_5.6", "UE_5.7", "UE_5.8")
for ($Index = 0; $Index -lt $ExpectedVersions.Count; $Index++) {
    if ((Split-Path $EngineRoots[$Index] -Leaf) -ne $ExpectedVersions[$Index]) {
        throw "Expected $($ExpectedVersions[$Index]) at argument $($Index + 1): $($EngineRoots[$Index])"
    }
}

$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
Push-Location $RepositoryRoot
try {
    $WorkingTreeChanges = & git status --porcelain --untracked-files=all
    if ($LASTEXITCODE -ne 0) { throw "Git working tree check failed." }
    if ($WorkingTreeChanges -and -not $AllowDirty) {
        throw "Release certification requires a clean working tree. Pass -AllowDirty for development validation."
    }

    py -3 -m unittest `
        Tests\Packaging\test_package_fab.py `
        Tests\Release\test_check_release.py `
        Tests\Release\test_record_release_evidence.py
    if ($LASTEXITCODE -ne 0) { throw "Python tests failed." }

    py -3 Tools\Release\check_release.py
    if ($LASTEXITCODE -ne 0) { throw "Release identity check failed." }

    py -3 Tools\Packaging\package_fab.py --check
    if ($LASTEXITCODE -ne 0) { throw "Fab source validation failed." }

    git diff --check
    if ($LASTEXITCODE -ne 0) { throw "Git whitespace validation failed." }

    $ReleaseFiles = & git ls-files -co --exclude-standard
    if ($LASTEXITCODE -ne 0) { throw "Release file discovery failed." }
    $ReleaseFiles = $ReleaseFiles | Where-Object {
        $_ -notlike "Build/*" -and
        $_ -notlike "Binaries/*" -and
        $_ -notlike "Intermediate/*" -and
        $_ -notlike "Tools/Release/run-local-release-gate.*"
    }
    $Markers = Select-String -LiteralPath $ReleaseFiles -Pattern "co-authored-by:|generated (with|by)|chatgpt|openai|claude|copilot|delve|seamless|robust|leverage|comprehensive|streamline|\u2014|\u2013"
    if ($Markers) {
        $Markers
        throw "Release text contains an attribution or generated-content marker."
    }

    $FabValidationRoot = Join-Path ([System.IO.Path]::GetTempPath()) "DUFab"
    & Tools\Release\verify-fab-artifacts.ps1 `
        -EngineRoots $EngineRoots `
        -ValidationRoot $FabValidationRoot `
        -AllowDirty:$AllowDirty `
        -MaxParallelActions $WindowsMaxParallelActions

    for ($Index = 0; $Index -lt $EngineRoots.Count; $Index++) {
        & Tests\RuntimeHost\Scripts\run-windows.ps1 `
            -EngineRoot $EngineRoots[$Index] `
            -ClientConfiguration Development `
            -MaxParallelActions $WindowsMaxParallelActions `
            -StompMalloc:($Index -eq 2)
    }

    if ($IncludeLinux) {
        if (-not $LinuxPackageBase) {
            $LinuxPackageBase = Join-Path ([System.IO.Path]::GetPathRoot($RepositoryRoot)) "DULinux"
        }
        $LinuxToolchainVersions = @(
            "v25_clang-18.1.0-rockylinux8",
            "v26_clang-20.1.8-rockylinux8",
            "v26_clang-20.1.8-rockylinux8"
        )
        $PreviousLinuxToolchainRoot = $env:LINUX_MULTIARCH_ROOT
        $PreviousMaxParallelActions = $env:UnrealBuildTool_BuildConfiguration__MaxParallelActions
        $PreviousAllowUbaExecutor = $env:UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor
        $env:UnrealBuildTool_BuildConfiguration__MaxParallelActions = "$WindowsMaxParallelActions"
        $env:UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor = "false"
        try {
            for ($Index = 0; $Index -lt $EngineRoots.Count; $Index++) {
                $LinuxToolchainRoot = Join-Path $LinuxToolchainBase $LinuxToolchainVersions[$Index]
                if (-not (Test-Path $LinuxToolchainRoot -PathType Container)) {
                    throw "Linux toolchain not found: $LinuxToolchainRoot"
                }

                $env:LINUX_MULTIARCH_ROOT = "$LinuxToolchainRoot\"
                $RunUAT = Join-Path $EngineRoots[$Index] "Engine\Build\BatchFiles\RunUAT.bat"
                $LinuxPackage = Join-Path $LinuxPackageBase $ExpectedVersions[$Index]
                Remove-Item $LinuxPackage -Recurse -Force -ErrorAction SilentlyContinue
                & $RunUAT BuildPlugin `
                    "-Plugin=$(Join-Path $RepositoryRoot 'DirectiveUtilities.uplugin')" `
                    "-Package=$LinuxPackage" `
                    "-TargetPlatforms=Linux" `
                    -Rocket
                if ($LASTEXITCODE -ne 0) {
                    throw "Linux plugin build failed for $($ExpectedVersions[$Index])."
                }
            }
        }
        finally {
            $env:LINUX_MULTIARCH_ROOT = $PreviousLinuxToolchainRoot
            if ($null -eq $PreviousMaxParallelActions) {
                Remove-Item Env:\UnrealBuildTool_BuildConfiguration__MaxParallelActions -ErrorAction SilentlyContinue
            }
            else {
                $env:UnrealBuildTool_BuildConfiguration__MaxParallelActions = $PreviousMaxParallelActions
            }
            if ($null -eq $PreviousAllowUbaExecutor) {
                Remove-Item Env:\UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor -ErrorAction SilentlyContinue
            }
            else {
                $env:UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor = $PreviousAllowUbaExecutor
            }
        }
    }

    $PerformanceRoot = Join-Path $RepositoryRoot "Build\Performance\ReleaseGate"
    $PerformanceProject = Join-Path $RepositoryRoot "Build\RuntimeHost\UE_5.8\Development\Project\DirectiveUtilitiesRuntimeHost.uproject"
    $PerformanceWarmup = Join-Path $PerformanceRoot "warmup.csv"
    $PerformanceBaseline = Join-Path $PerformanceRoot "baseline.csv"
    $PerformanceCandidate = Join-Path $PerformanceRoot "candidate.csv"
    $PerformanceCandidateAttempt1 = Join-Path $PerformanceRoot "candidate-attempt-1.csv"
    $PerformanceCandidateAttempt2 = Join-Path $PerformanceRoot "candidate-attempt-2.csv"
    $PerformanceCandidateAttempt3 = Join-Path $PerformanceRoot "candidate-attempt-3.csv"
    & Tests\Performance\run-runtime-benchmarks.ps1 `
        -EngineRoot $EngineRoots[2] -ProjectFile $PerformanceProject -OutputFile $PerformanceWarmup
    $PerformanceBaselineRuns = 1..3 | ForEach-Object {
        $BaselineRun = Join-Path $PerformanceRoot "baseline-run-$_.csv"
        & Tests\Performance\run-runtime-benchmarks.ps1 `
            -EngineRoot $EngineRoots[2] -ProjectFile $PerformanceProject -OutputFile $BaselineRun
        $BaselineRun
    }
    & py -3 Tools\Release\aggregate_performance_baselines.py `
        --output $PerformanceBaseline @PerformanceBaselineRuns
    if ($LASTEXITCODE -ne 0) { throw "Performance baseline aggregation failed." }
    try {
        & Tests\Performance\run-runtime-benchmarks.ps1 `
            -EngineRoot $EngineRoots[2] -ProjectFile $PerformanceProject -OutputFile $PerformanceCandidateAttempt1 `
            -BaselineFile $PerformanceBaseline
        Copy-Item $PerformanceCandidateAttempt1 $PerformanceCandidate -Force
    }
    catch {
        Write-Warning "The first performance candidate failed. Two clean retries are required."
        & Tests\Performance\run-runtime-benchmarks.ps1 `
            -EngineRoot $EngineRoots[2] -ProjectFile $PerformanceProject -OutputFile $PerformanceCandidateAttempt2 `
            -BaselineFile $PerformanceBaseline
        & Tests\Performance\run-runtime-benchmarks.ps1 `
            -EngineRoot $EngineRoots[2] -ProjectFile $PerformanceProject -OutputFile $PerformanceCandidateAttempt3 `
            -BaselineFile $PerformanceBaseline
        Copy-Item $PerformanceCandidateAttempt3 $PerformanceCandidate -Force
    }

    & Tests\RuntimeHost\Scripts\run-windows.ps1 `
        -EngineRoot $EngineRoots[2] `
        -ClientConfiguration Shipping `
        -MaxParallelActions $WindowsMaxParallelActions
    $EvidenceArguments = @(
        "Tools\Release\record_release_evidence.py",
        "--output",
        (Join-Path $RepositoryRoot "Build\ReleaseEvidence\Windows\manifest.json"),
        "--fab-validation-root",
        $FabValidationRoot
    )
    if ($AllowDirty) {
        $EvidenceArguments += "--allow-dirty"
    }
    if ($IncludeLinux) {
        $EvidenceArguments += @("--linux-package-base", $LinuxPackageBase)
    }
    & py -3 @EvidenceArguments
    if ($LASTEXITCODE -ne 0) { throw "Release evidence recording failed." }
    Write-Host "Local release gate passed."
}
finally {
    Pop-Location
}
