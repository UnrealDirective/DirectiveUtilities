param(
    [Parameter(Mandatory = $true)]
    [string]$EngineRoot,

    [Parameter(Mandatory = $true)]
    [string]$ProjectFile,

    [Parameter(Mandatory = $true)]
    [string]$OutputFile,

    [string]$BaselineFile = ""
)

$ErrorActionPreference = "Stop"

$EditorCommand = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path $EditorCommand -PathType Leaf)) {
    throw "Unreal Editor command not found: $EditorCommand"
}
if (-not (Test-Path $ProjectFile -PathType Leaf)) {
    throw "Project file not found: $ProjectFile"
}
if ($BaselineFile -and -not (Test-Path $BaselineFile -PathType Leaf)) {
    throw "Baseline file not found: $BaselineFile"
}

$OutputDirectory = Split-Path $OutputFile -Parent
if ($OutputDirectory) {
    New-Item $OutputDirectory -ItemType Directory -Force | Out-Null
}
$LogFile = [System.IO.Path]::ChangeExtension($OutputFile, ".log")
$Arguments = @(
    $ProjectFile,
    '-ExecCmds=Automation RunTests Performance.DirectiveUtilities.Runtime; Quit',
    '-TestExit=Automation Test Queue Empty',
    "-DirectiveUtilitiesPerfOutput=$OutputFile",
    "-abslog=$LogFile",
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nosound',
    '-NullRHI'
)
if ($BaselineFile) {
    $Arguments += "-DirectiveUtilitiesPerfBaseline=$BaselineFile"
}

& $EditorCommand $Arguments
if ($LASTEXITCODE -ne 0) {
    throw "Runtime performance suite failed. Log: $LogFile"
}
if (-not (Test-Path $OutputFile -PathType Leaf)) {
    throw "Runtime performance results were not generated: $OutputFile"
}
if (-not (Select-String -Path $LogFile -Pattern 'Test Completed\. Result=\{Success\} Name=\{Runtime\} Path=\{Performance\.DirectiveUtilities\.Runtime\}' -Quiet)) {
    throw "Runtime performance suite did not complete successfully. Log: $LogFile"
}

Write-Host "Runtime performance results: $OutputFile"
Write-Host "Automation log: $LogFile"
