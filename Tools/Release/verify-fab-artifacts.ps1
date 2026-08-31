param(
    [Parameter(Mandatory = $true)]
    [string[]]$EngineRoots,

    [string]$ValidationRoot = "",

    [switch]$AllowDirty,

    [ValidateRange(1, 128)]
    [int]$MaxParallelActions = 8
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if ($EngineRoots.Count -ne 3) {
    throw "Provide the UE 5.6, 5.7, and 5.8 engine roots in that order."
}

$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$PluginDescriptor = Get-Content (Join-Path $RepositoryRoot "DirectiveUtilities.uplugin") -Raw | ConvertFrom-Json
$PluginVersion = $PluginDescriptor.VersionName
$FabRoot = Join-Path $RepositoryRoot "Build\Fab"
if (-not $ValidationRoot) {
    $ValidationRoot = Join-Path ([System.IO.Path]::GetTempPath()) "DUFab"
}
$ValidationRoot = [System.IO.Path]::GetFullPath($ValidationRoot)
$PackageArguments = @(
    (Join-Path $RepositoryRoot "Tools\Packaging\package_fab.py"),
    "--output-dir",
    $FabRoot
)
if ($AllowDirty) {
    $PackageArguments += "--allow-dirty"
}

New-Item $FabRoot -ItemType Directory -Force | Out-Null
New-Item $ValidationRoot -ItemType Directory -Force | Out-Null
& py -3 @PackageArguments
if ($LASTEXITCODE -ne 0) {
    throw "Fab archive generation failed."
}

$EngineVersions = @("5.6", "5.7", "5.8")
$PreviousMaxParallelActions = $env:UnrealBuildTool_BuildConfiguration__MaxParallelActions
$PreviousAllowUbaExecutor = $env:UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor
$env:UnrealBuildTool_BuildConfiguration__MaxParallelActions = "$MaxParallelActions"
$env:UnrealBuildTool_BuildConfiguration__bAllowUBAExecutor = "false"

try {
    for ($Index = 0; $Index -lt $EngineRoots.Count; $Index++) {
        $EngineVersion = $EngineVersions[$Index]
        $RunUAT = Join-Path $EngineRoots[$Index] "Engine\Build\BatchFiles\RunUAT.bat"
        $ArchivePath = Join-Path $FabRoot "DirectiveUtilities-$PluginVersion-UE$EngineVersion-Fab.zip"
        $VersionRoot = Join-Path $ValidationRoot "UE_$EngineVersion"
        $ExtractRoot = Join-Path $VersionRoot "Extracted"
        $PackageRoot = Join-Path $VersionRoot "Package"

        if (-not (Test-Path $RunUAT -PathType Leaf)) {
            throw "RunUAT not found: $RunUAT"
        }
        if (-not (Test-Path $ArchivePath -PathType Leaf)) {
            throw "Fab archive not found: $ArchivePath"
        }

        Remove-Item $ExtractRoot, $PackageRoot -Recurse -Force -ErrorAction SilentlyContinue
        Expand-Archive -LiteralPath $ArchivePath -DestinationPath $ExtractRoot
        $PluginPath = Join-Path $ExtractRoot "DirectiveUtilities\DirectiveUtilities.uplugin"
        if (-not (Test-Path $PluginPath -PathType Leaf)) {
            throw "Extracted plugin descriptor not found: $PluginPath"
        }

        & $RunUAT BuildPlugin `
            "-Plugin=$PluginPath" `
            "-Package=$PackageRoot" `
            "-TargetPlatforms=Win64" `
            -Rocket
        if ($LASTEXITCODE -ne 0) {
            throw "Extracted Fab plugin build failed for UE $EngineVersion."
        }
    }
}
finally {
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

Write-Host "Generated Fab archives compiled successfully."
