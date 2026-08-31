param(
    [Parameter(Mandatory = $true)]
    [string]$EngineRoot,

    [ValidateSet("Development", "Shipping")]
    [string]$ClientConfiguration = "Development",

    [ValidateRange(1, 86400)]
    [int]$TestTimeoutSeconds = 1800,

    [ValidateRange(1, 128)]
    [int]$MaxParallelActions = 8,

    [switch]$StompMalloc
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$ScriptRoot = $PSScriptRoot
$RepositoryRoot = (Resolve-Path (Join-Path $ScriptRoot "..\..\..")).Path
$EngineVersion = Split-Path $EngineRoot -Leaf
$WorkRoot = Join-Path $RepositoryRoot "Build\RuntimeHost\$EngineVersion\$ClientConfiguration"
$ProjectRoot = Join-Path $WorkRoot "Project"
$ProjectFile = Join-Path $ProjectRoot "DirectiveUtilitiesRuntimeHost.uproject"
$PluginRoot = Join-Path $ProjectRoot "Plugins\DirectiveUtilities"
$PluginDescriptor = Join-Path $PluginRoot "DirectiveUtilities.uplugin"
$RuntimeTestModule = Join-Path $ProjectRoot "Source\DirectiveUtilitiesRuntimeHostTests"
$RuntimeTestSourceRoot = Join-Path $RepositoryRoot "Source\DirectiveUtilitiesTests"
$ArchiveRoot = Join-Path $WorkRoot "Archive"
$ReportRoot = Join-Path $WorkRoot "Reports"
$PerformanceRoot = Join-Path $WorkRoot "Performance"
$BuildScript = Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat"
$RunUAT = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"
$EditorCommand = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$LongestActionPath = Join-Path $ProjectRoot "Plugins\DirectiveUtilities\Intermediate\Build\Win64\x64\UnrealEditor\Development\DirectiveUtilitiesBlueprintNodes\UnrealEditor-DirectiveUtilitiesBlueprintNodes.dll.rsp"
$ExpectedEditorTestCount = 61
$ExpectedPackagedTestCount = 42

function Wait-TestProcess {
    param(
        [Parameter(Mandatory = $true)]
        [System.Diagnostics.Process]$Process,
        [Parameter(Mandatory = $true)]
        [string]$Label,
        [Parameter(Mandatory = $true)]
        [string]$LogPath
    )

    if (-not $Process.WaitForExit($TestTimeoutSeconds * 1000)) {
        Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
        $Process.WaitForExit()
        throw "$Label timed out after $TestTimeoutSeconds seconds. Log: $LogPath"
    }
    $Process.WaitForExit()
    if ($Process.ExitCode -ne 0) {
        if (Test-Path $LogPath -PathType Leaf) {
            Get-Content $LogPath -Tail 100
        }
        throw "$Label failed with exit code $($Process.ExitCode). Log: $LogPath"
    }
}

function Start-TestProcess {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,
        [Parameter(Mandatory = $true)]
        [string]$ArgumentLine,
        [Parameter(Mandatory = $true)]
        [string]$WorkingDirectory
    )

    $StartInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $StartInfo.FileName = $FilePath
    $StartInfo.Arguments = $ArgumentLine
    $StartInfo.WorkingDirectory = $WorkingDirectory
    $StartInfo.UseShellExecute = $false
    $Process = [System.Diagnostics.Process]::new()
    $Process.StartInfo = $StartInfo
    if (-not $Process.Start()) {
        throw "Failed to start process: $FilePath"
    }
    return $Process
}

function Assert-CleanAutomationReport {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ReportPath,
        [Parameter(Mandatory = $true)]
        [int]$ExpectedCount,
        [Parameter(Mandatory = $true)]
        [string]$Label
    )

    if (-not (Test-Path $ReportPath -PathType Leaf)) {
        throw "$Label automation report was not generated: $ReportPath"
    }
    $Report = Get-Content $ReportPath -Raw | ConvertFrom-Json
    if ($Report.failed -ne 0 -or $Report.succeededWithWarnings -ne 0 -or $Report.notRun -ne 0) {
        throw "$Label automation tests failed, warned, or were skipped. Report: $ReportPath"
    }
    if ($Report.succeeded -ne $ExpectedCount -or @($Report.tests).Count -ne $ExpectedCount) {
        throw "$Label expected $ExpectedCount tests but report contains $(@($Report.tests).Count) with $($Report.succeeded) successes. Report: $ReportPath"
    }
    $BadStates = @($Report.tests | Where-Object { $_.state -ne "Success" })
    if ($BadStates.Count -ne 0) {
        throw "$Label report contains non-success test states. Report: $ReportPath"
    }
    $DuplicatePaths = @($Report.tests | Group-Object fullTestPath | Where-Object { $_.Count -ne 1 })
    if ($DuplicatePaths.Count -ne 0) {
        throw "$Label report contains duplicate test paths. Report: $ReportPath"
    }
}

if ($LongestActionPath.Length -ge 260) {
    throw "The Windows RuntimeHost path would exceed Unreal's 260-character action-path limit. Move the repository to a shorter path."
}

foreach ($RequiredFile in @($BuildScript, $RunUAT, $EditorCommand)) {
    if (-not (Test-Path $RequiredFile -PathType Leaf)) {
        throw "Required file not found: $RequiredFile"
    }
}

Remove-Item $WorkRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item $PluginRoot -ItemType Directory -Force | Out-Null
New-Item (Join-Path $ReportRoot "Editor") -ItemType Directory -Force | Out-Null
if ($ClientConfiguration -eq "Development") {
    New-Item (Join-Path $ReportRoot "Game") -ItemType Directory -Force | Out-Null
}

robocopy (Join-Path $RepositoryRoot "Tests\RuntimeHost") $ProjectRoot /E /XD Binaries Intermediate Saved Scripts Plugins | Out-Null
if ($LASTEXITCODE -ge 8) {
    throw "Failed to stage the runtime host project."
}

Copy-Item (Join-Path $RepositoryRoot "DirectiveUtilities.uplugin") $PluginDescriptor
foreach ($Directory in @("Source", "Config", "Resources")) {
    $SourceDirectory = Join-Path $RepositoryRoot $Directory
    if (-not (Test-Path $SourceDirectory -PathType Container)) {
        continue
    }

    robocopy $SourceDirectory (Join-Path $PluginRoot $Directory) /E /XD Binaries Intermediate | Out-Null
    if ($LASTEXITCODE -ge 8) {
        throw "Failed to stage plugin directory: $Directory"
    }
}

$RuntimeTestPrivateRoot = Join-Path $RuntimeTestModule "Private\Tests"
$RuntimeTestPublicRoot = Join-Path $RuntimeTestModule "Public\Tests"
New-Item $RuntimeTestPrivateRoot -ItemType Directory -Force | Out-Null
New-Item $RuntimeTestPublicRoot -ItemType Directory -Force | Out-Null
foreach ($TestSource in Get-Content (Join-Path $RepositoryRoot "Tests\RuntimeHost\RuntimeTestSources.txt")) {
    if (-not [string]::IsNullOrWhiteSpace($TestSource)) {
        Copy-Item (Join-Path $RuntimeTestSourceRoot "Private\Tests\$TestSource") $RuntimeTestPrivateRoot
    }
}
Copy-Item (Join-Path $RuntimeTestSourceRoot "Public\Tests\DirectiveUtilTestObject.h") $RuntimeTestPublicRoot
Copy-Item (Join-Path $RuntimeTestSourceRoot "Public\Tests\DirectiveUtilTestDataTableRows.h") $RuntimeTestPublicRoot

& $BuildScript DirectiveUtilitiesRuntimeHostEditor Win64 Development `
    "-Project=$ProjectFile" -WaitMutex -NoHotReload -ForceUnity -DisableAdaptiveUnity -NoUBA `
    "-MaxParallelActions=$MaxParallelActions"
if ($LASTEXITCODE -ne 0) {
    throw "Editor target build failed."
}

$EditorArguments = @(
    $ProjectFile,
    '-ExecCmds=Automation RunTests DirectiveUtilities; Quit',
    "-ReportExportPath=$(Join-Path $ReportRoot 'Editor')",
    '-TestExit=Automation Test Queue Empty',
    "-abslog=$(Join-Path $WorkRoot 'EditorTests.log')",
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nosound',
    '-NullRHI',
    '-NoEngineAnalytics',
    '-NoEpicPortal',
    '-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False'
)
if ($StompMalloc) {
    $EditorArguments += '-stompmalloc'
}
$EditorArgumentLine = ($EditorArguments | ForEach-Object { '"{0}"' -f $_ }) -join ' '
$EditorProcess = Start-TestProcess `
    -FilePath $EditorCommand `
    -ArgumentLine $EditorArgumentLine `
    -WorkingDirectory $ProjectRoot
Wait-TestProcess -Process $EditorProcess -Label "Editor automation" -LogPath (Join-Path $WorkRoot "EditorTests.log")

$PackageArguments = @(
    'BuildCookRun',
    "-project=$ProjectFile",
    '-noP4',
    '-platform=Win64',
    "-clientconfig=$ClientConfiguration",
    '-build',
    '-cook',
    '-stage',
    '-pak',
    '-skipzenstore',
    '-package',
    '-archive',
    "-archivedirectory=$ArchiveRoot",
    "-ubtargs=-NoUBA -MaxParallelActions=$MaxParallelActions",
    '-unattended',
    '-utf8output'
)
& $RunUAT @PackageArguments
if ($LASTEXITCODE -ne 0) {
    throw "Packaged game build failed."
}

$GameCommand = Get-ChildItem $ArchiveRoot -Filter "DirectiveUtilitiesRuntimeHost*.exe" -File -Recurse |
    Where-Object { $_.DirectoryName -like "*\Binaries\Win64" } |
    Select-Object -First 1
if (-not $GameCommand) {
    throw "Packaged game executable not found under $ArchiveRoot"
}

$GameLog = Join-Path $WorkRoot "GameTests.log"
$GameArguments = @(
    "-abslog=$GameLog",
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nosound',
    '-NullRHI',
    '-NoEngineAnalytics',
    '-NoEpicPortal'
)
if ($StompMalloc -and $ClientConfiguration -eq "Development") {
    $GameArguments += '-stompmalloc'
}
if ($ClientConfiguration -eq "Shipping") {
    New-Item $PerformanceRoot -ItemType Directory -Force | Out-Null
    $AppendOutput = Join-Path $PerformanceRoot "shipping-append-comparison.csv"
    $SmokeOutput = Join-Path $PerformanceRoot "shipping-smoke.csv"
    $Revision = ""
    if (Get-Command git -ErrorAction SilentlyContinue) {
        $Revision = (& git -C $RepositoryRoot rev-parse HEAD 2>$null)
        if ($Revision -and (& git -C $RepositoryRoot status --porcelain 2>$null)) {
            $Revision = "$Revision-dirty"
        }
    }
    $GameArguments += @(
        "-DirectiveUtilitiesAppendShippingBenchmarkOutput=$AppendOutput",
        "-DirectiveUtilitiesShippingSmokeOutput=$SmokeOutput",
        "-DirectiveUtilitiesPerfRevision=$Revision"
    )
} else {
    $GameArguments += @(
        '-ExecCmds=Automation RunTests DirectiveUtilities; Quit',
        '-TestExit=Automation Test Queue Empty',
        "-ReportExportPath=$(Join-Path $ReportRoot 'Game')"
    )
}
$GameCommandPath = $GameCommand.FullName
$GameArgumentLine = ($GameArguments | ForEach-Object { '"{0}"' -f $_ }) -join ' '
$GameProcess = Start-TestProcess `
    -FilePath $GameCommandPath `
    -ArgumentLine $GameArgumentLine `
    -WorkingDirectory $GameCommand.DirectoryName
Wait-TestProcess -Process $GameProcess -Label "Packaged game" -LogPath $GameLog

$EditorReportPath = Join-Path $ReportRoot "Editor\index.json"
Assert-CleanAutomationReport -ReportPath $EditorReportPath -ExpectedCount $ExpectedEditorTestCount -Label "Editor"

if ($ClientConfiguration -eq "Shipping") {
    if (-not (Test-Path $AppendOutput -PathType Leaf)) {
        throw "Packaged Shipping append result was not generated: $AppendOutput"
    }
    if (-not (Select-String -Path $AppendOutput -Pattern '^#configuration,Shipping$' -Quiet)) {
        throw "Packaged benchmark did not report a Shipping configuration: $AppendOutput"
    }
    foreach ($ElementType in @("bool", "int32", "float", "FVector", "FString", "UObject")) {
        if ((Select-String -Path $AppendOutput -Pattern "^$ElementType,").Count -ne 10) {
            throw "Packaged benchmark did not report all $ElementType scenarios: $AppendOutput"
        }
    }
    if (-not (Test-Path $SmokeOutput -PathType Leaf)) {
        throw "Packaged Shipping smoke result was not generated: $SmokeOutput"
    }
    if (-not (Select-String -Path $SmokeOutput -Pattern '^#configuration,Shipping$' -Quiet)) {
        throw "Packaged smoke test did not report a Shipping configuration: $SmokeOutput"
    }
    if (Select-String -Path $SmokeOutput -Pattern ',false$' -Quiet) {
        throw "Packaged Shipping smoke test reported a failure: $SmokeOutput"
    }
    if ((Select-String -Path $SmokeOutput -Pattern ',true$').Count -ne 15) {
        throw "Packaged Shipping smoke test did not report every check: $SmokeOutput"
    }
} else {
    if (-not (Test-Path $GameLog -PathType Leaf)) {
        throw "Packaged game automation log was not generated: $GameLog"
    }
    if (-not (Select-String -Path $GameLog -Pattern 'TEST COMPLETE\. EXIT CODE: 0' -Quiet)) {
        throw "Packaged game automation did not finish cleanly. Log: $GameLog"
    }
    Assert-CleanAutomationReport `
        -ReportPath (Join-Path $ReportRoot "Game\index.json") `
        -ExpectedCount $ExpectedPackagedTestCount `
        -Label "Packaged game"
}

Write-Host "Reports: $ReportRoot"
if ($ClientConfiguration -eq "Shipping") {
    Write-Host "Editor tests and packaged Shipping smoke and benchmark passed for $EngineVersion."
    Write-Host "Shipping performance results: $PerformanceRoot"
} else {
    Write-Host "Editor and packaged game tests passed for $EngineVersion."
    Write-Host "Packaged game log: $GameLog"
}

# Best-effort metadata commands (notably git in a source archive) may leave a
# stale non-zero native exit code even though every required gate passed.
exit 0
