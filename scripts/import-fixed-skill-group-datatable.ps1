<#
Imports the FixedSkillGroups source CSV into a Unreal DataTable asset.

This script is guarded: use -CommandOnly for CI-safe validation. Running without
-CommandOnly launches Unreal Editor commandlet mode and writes only the target
/Game/DBA/Data/Tables/DT_FixedSkillGroups asset.
#>

[CmdletBinding()]
param(
    [string]$UnrealRoot = $env:UNREAL_ENGINE_ROOT,
    [string]$ProjectPath = "",
    [string]$CsvPath = "",
    [string]$AssetPackagePath = "/Game/DBA/Data/Tables/DT_FixedSkillGroups",
    [string]$RowStructPath = "/Script/DivineBeastsArena.DBAZodiacElementFixedSkillGroupRow",
    [switch]$CommandOnly
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$pythonScript = Join-Path $repoRoot "scripts\unreal\import_fixed_skill_group_datatable.py"

function New-TextFromCodePoints {
    param([Parameter(Mandatory = $true)][object[]]$CodePoints)

    return -join ($CodePoints | ForEach-Object { [char]([int]$_) })
}

function Write-Step {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ("[fixed-skill-group-import] {0}" -f $Message) -ForegroundColor Cyan
}

$projectFileMissingMessage = New-TextFromCodePoints @(0x9879, 0x76EE, 0x6587, 0x4EF6, 0x4E0D, 0x5B58, 0x5728)
$sourceCsvMissingMessage = (New-TextFromCodePoints @(0x6E90)) + " CSV " + (New-TextFromCodePoints @(0x4E0D, 0x5B58, 0x5728))
$pythonScriptMissingMessage = "Unreal " + (New-TextFromCodePoints @(0x5BFC, 0x5165)) + " Python " + (New-TextFromCodePoints @(0x811A, 0x672C, 0x4E0D, 0x5B58, 0x5728))
$editorCmdMissingMessage = "UnrealEditor-Cmd.exe " + (New-TextFromCodePoints @(0x4E0D, 0x5B58, 0x5728))
$willWriteAssetMessage = (New-TextFromCodePoints @(0x5373, 0x5C06, 0x5199, 0x5165)) + " Unreal " + (New-TextFromCodePoints @(0x8D44, 0x4EA7))
$sourceCsvStepMessage = (New-TextFromCodePoints @(0x6E90)) + " CSV"
$importFailedMessage = (New-TextFromCodePoints @(0x56FA, 0x5B9A, 0x6280, 0x80FD, 0x7EC4)) + " DataTable " + (New-TextFromCodePoints @(0x5BFC, 0x5165, 0x5931, 0x8D25)) + (New-TextFromCodePoints @(0xFF0C, 0x9000, 0x51FA, 0x7801))
$assetNotCreatedMessage = (New-TextFromCodePoints @(0x56FA, 0x5B9A, 0x6280, 0x80FD, 0x7EC4)) + " DataTable " + (New-TextFromCodePoints @(0x5BFC, 0x5165, 0x672A, 0x521B, 0x5EFA, 0x76EE, 0x6807, 0x8D44, 0x4EA7))
$completedImportMessage = (New-TextFromCodePoints @(0x5DF2, 0x5B8C, 0x6210, 0x5BFC, 0x5165))

if ([string]::IsNullOrWhiteSpace($UnrealRoot)) {
    $UnrealRoot = "D:\UnrealEngine-5.8.0-release"
}

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Join-Path $repoRoot "DBA_GameClient\DivineBeastsArena.uproject"
}

if ([string]::IsNullOrWhiteSpace($CsvPath)) {
    $CsvPath = Join-Path $repoRoot "DBA_GameClient\Content\DBA\Data\Tables\Source\DT_FixedSkillGroups.csv"
}

$resolvedProjectPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($ProjectPath)
if (-not (Test-Path -LiteralPath $resolvedProjectPath)) {
    throw "${projectFileMissingMessage}: $resolvedProjectPath"
}

$resolvedCsvPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($CsvPath)
if (-not (Test-Path -LiteralPath $resolvedCsvPath)) {
    throw "${sourceCsvMissingMessage}: $resolvedCsvPath"
}

if (-not (Test-Path -LiteralPath $pythonScript)) {
    throw "${pythonScriptMissingMessage}: $pythonScript"
}

$editorCmd = Join-Path $UnrealRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path -LiteralPath $editorCmd)) {
    throw "${editorCmdMissingMessage}: $editorCmd"
}

$arguments = @(
    $resolvedProjectPath,
    "-Unattended",
    "-NullRHI",
    "-NoSound",
    "-NoSplash",
    "-ExecutePythonScript=$pythonScript"
)

$environmentVariables = [ordered]@{
    DBA_FIXED_SKILL_GROUP_CSV = $resolvedCsvPath
    DBA_FIXED_SKILL_GROUP_ASSET = $AssetPackagePath
    DBA_FIXED_SKILL_GROUP_ROW_STRUCT = $RowStructPath
}

$assetFilePath = Join-Path $repoRoot "DBA_GameClient\Content\DBA\Data\Tables\DT_FixedSkillGroups.uasset"

$command = [PSCustomObject]@{
    FilePath = $editorCmd
    WorkingDirectory = $repoRoot
    Arguments = $arguments
    EnvironmentVariables = $environmentVariables
    CsvPath = $resolvedCsvPath
    AssetPackagePath = $AssetPackagePath
    AssetFilePath = $assetFilePath
    RowStructPath = $RowStructPath
    PythonScript = $pythonScript
    WillWriteAsset = -not $CommandOnly
}

if ($CommandOnly) {
    return $command
}

Write-Step "${willWriteAssetMessage}: $AssetPackagePath"
Write-Step "${sourceCsvStepMessage}: $resolvedCsvPath"

Push-Location $repoRoot
$previousCsv = $env:DBA_FIXED_SKILL_GROUP_CSV
$previousAsset = $env:DBA_FIXED_SKILL_GROUP_ASSET
$previousRowStruct = $env:DBA_FIXED_SKILL_GROUP_ROW_STRUCT
try {
    $env:DBA_FIXED_SKILL_GROUP_CSV = $environmentVariables.DBA_FIXED_SKILL_GROUP_CSV
    $env:DBA_FIXED_SKILL_GROUP_ASSET = $environmentVariables.DBA_FIXED_SKILL_GROUP_ASSET
    $env:DBA_FIXED_SKILL_GROUP_ROW_STRUCT = $environmentVariables.DBA_FIXED_SKILL_GROUP_ROW_STRUCT

    & $editorCmd @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "${importFailedMessage}: $LASTEXITCODE"
    }

    if (-not (Test-Path -LiteralPath $assetFilePath)) {
        throw "${assetNotCreatedMessage}: $assetFilePath"
    }
}
finally {
    $env:DBA_FIXED_SKILL_GROUP_CSV = $previousCsv
    $env:DBA_FIXED_SKILL_GROUP_ASSET = $previousAsset
    $env:DBA_FIXED_SKILL_GROUP_ROW_STRUCT = $previousRowStruct
    Pop-Location
}

Write-Step "${completedImportMessage}: $AssetPackagePath"
return $command
