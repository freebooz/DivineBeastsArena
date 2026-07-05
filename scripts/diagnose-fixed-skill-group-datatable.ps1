<#
Read-only diagnostic for the real FixedSkillGroups DataTable release blocker.

The script does not create, save, delete, or edit Unreal assets. It checks the
expected /Game package location and can run the focused Editor automation test
when the asset and editor command are available.
#>

[CmdletBinding()]
param(
    [string]$UnrealRoot = $env:UNREAL_ENGINE_ROOT,
    [string]$ProjectPath = "",
    [string]$TestFilter = "DivineBeastsArena.GameDBA.Data.FixedSkillGroup.AssetRows",
    [string]$AssetPackagePath = "/Game/DBA/Data/Tables/DT_FixedSkillGroups",
    [switch]$CommandOnly
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][object[]]$CodePoints)

    return -join ($CodePoints | ForEach-Object { [char]([int]$_) })
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[fixed-skill-group-datatable] {0}" -f $Message) -ForegroundColor Cyan
}

$nonGamePackageMessage = New-TextFromCodePoints @(0x4EC5, 0x652F, 0x6301, 0x20, 0x2F, 0x47, 0x61, 0x6D, 0x65, 0x20, 0x5305, 0x8DEF, 0x5F84)
$projectFileMissingMessage = New-TextFromCodePoints @(0x9879, 0x76EE, 0x6587, 0x4EF6, 0x4E0D, 0x5B58, 0x5728)
$editorCmdMissingMessage = "UnrealEditor-Cmd.exe " + (New-TextFromCodePoints @(0x4E0D, 0x5B58, 0x5728))
$checkingAssetFileMessage = New-TextFromCodePoints @(0x6B63, 0x5728, 0x68C0, 0x67E5, 0x8D44, 0x4EA7, 0x6587, 0x4EF6)
$assetFileMissingMessage = New-TextFromCodePoints @(0x56FA, 0x5B9A, 0x6280, 0x80FD, 0x7EC4, 0x20, 0x44, 0x61, 0x74, 0x61, 0x54, 0x61, 0x62, 0x6C, 0x65, 0x20, 0x8D44, 0x4EA7, 0x4E0D, 0x5B58, 0x5728)
$runningAutomationMessage = New-TextFromCodePoints @(0x6B63, 0x5728, 0x8FD0, 0x884C, 0x81EA, 0x52A8, 0x5316, 0x9A8C, 0x8BC1)
$automationFailedMessage = New-TextFromCodePoints @(0x56FA, 0x5B9A, 0x6280, 0x80FD, 0x7EC4, 0x20, 0x44, 0x61, 0x74, 0x61, 0x54, 0x61, 0x62, 0x6C, 0x65, 0x20, 0x81EA, 0x52A8, 0x5316, 0x9A8C, 0x8BC1, 0x5931, 0x8D25)
$completedMessage = New-TextFromCodePoints @(0x5DF2, 0x5B8C, 0x6210)

function Convert-GamePackagePathToContentFile {
    param([Parameter(Mandatory = $true)][string]$PackagePath)

    if (-not $PackagePath.StartsWith("/Game/")) {
        throw "${nonGamePackageMessage}: $PackagePath"
    }

    $relativePath = $PackagePath.Substring("/Game/".Length).Replace("/", [System.IO.Path]::DirectorySeparatorChar)
    return Join-Path $repoRoot ("DBA_GameClient\Content\{0}.uasset" -f $relativePath)
}

if ([string]::IsNullOrWhiteSpace($UnrealRoot)) {
    $UnrealRoot = "D:\UnrealEngine-5.8.0-release"
}

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject"
}

$resolvedProjectPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ProjectPath)
if (-not (Test-Path -LiteralPath $resolvedProjectPath)) {
    throw "${projectFileMissingMessage}: $resolvedProjectPath"
}

$editorCmd = Join-Path $UnrealRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path -LiteralPath $editorCmd)) {
    throw "${editorCmdMissingMessage}: $editorCmd"
}

$assetFilePath = Convert-GamePackagePathToContentFile -PackagePath $AssetPackagePath
$arguments = @(
    $resolvedProjectPath,
    "-Unattended",
    "-NullRHI",
    "-NoSound",
    "-NoSplash",
    "-ExecCmds=Automation RunTests $TestFilter; Quit",
    "-TestExit=Automation Test Queue Empty"
)

$command = [PSCustomObject]@{
    FilePath = $editorCmd
    WorkingDirectory = $repoRoot
    Arguments = $arguments
    AssetPackagePath = $AssetPackagePath
    AssetFilePath = $assetFilePath
    AssetFileExists = Test-Path -LiteralPath $assetFilePath
    TestFilter = $TestFilter
}

if ($CommandOnly) {
    return $command
}

Write-Step ("{0}: {1}" -f $checkingAssetFileMessage, $assetFilePath)
if (-not $command.AssetFileExists) {
    throw "${assetFileMissingMessage}: $AssetPackagePath ($assetFilePath)"
}

Write-Step ("{0}: {1}" -f $runningAutomationMessage, $TestFilter)
Push-Location $repoRoot
try {
    & $editorCmd @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "${automationFailedMessage}: $LASTEXITCODE"
    }
}
finally {
    Pop-Location
}

Write-Step ("{0}: {1}" -f $completedMessage, $TestFilter)
return $command
