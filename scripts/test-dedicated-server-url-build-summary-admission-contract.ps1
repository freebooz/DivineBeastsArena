<#
Validates the Dedicated Server URL build-summary admission contract.

This is a lightweight source contract test. It does not launch Unreal or edit
assets; it verifies that session travel URL build fields are parsed, normalized,
and rejected before Runtime player-joined reporting.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

function New-CodePointText {
    param(
        [Parameter(Mandatory = $true)][int[]]$CodePoints
    )

    return -join ($CodePoints | ForEach-Object { [char]$_ })
}

function Assert-FileContains {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][string[]]$RequiredSymbols
    )

    $fullPath = Join-Path $repoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $fullPath)) {
        throw "Required file is missing: $RelativePath"
    }

    $content = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath
    $missing = @($RequiredSymbols | Where-Object { $content -notmatch [regex]::Escape($_) })
    if ($missing.Count -gt 0) {
        throw "$RelativePath is missing Dedicated Server URL build-summary admission symbols: $($missing -join ', ')"
    }
}

$runtimePlayerJoinedText = New-CodePointText @(0x8FD0, 0x884C, 0x65F6, 0x73A9, 0x5BB6, 0x52A0, 0x5165)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Framework\DBAUrlOptions.h" @(
    "ExtractUrlOption",
    "TryExtractCharacterBuildSummary",
    "TryExtractTeamId",
    "FDBACharacterBuildSummary"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAUrlOptions.cpp" @(
    "FGenericPlatformHttp::UrlDecode",
    "ParseStableZodiacName",
    "ParseStableElementName",
    "ParseStableFiveCampName",
    "DBACharacterBuild::MakeFixedSkillGroupId",
    "OutSummary.IsValid()",
    "FixedSkillGroupId == ExpectedFixedSkillGroupId",
    'ExtractUrlOption(Options, TEXT("DBATeamId"))',
    'ExtractUrlOption(Options, TEXT("TeamId"))',
    "OutTeamId"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAGameModeBase.cpp" @(
    "TryExtractCharacterBuildSummary",
    "TryExtractTeamId",
    "AdmissionBuildSummary",
    "BackendTeamId <= 0",
    "BuildBackendRuntimeTeamName(BackendTeamId)",
    "ToStableZodiacName",
    "ToStableElementName",
    "ToStableFiveCampName",
    "RuntimeService->NotifyPlayerJoined",
    $runtimePlayerJoinedText
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAUrlOptionsTests.cpp" @(
    "FDBAUrlOptionsDecodeTest",
    "FDBAUrlOptionsBuildSummaryAdmissionTest",
    "ValidatesDedicatedServerBuildSummary",
    "DBAZodiac=Rat",
    "DBAElement=Water",
    "DBAFiveCamp=East",
    "DBAFixedSkillGroupId=Rat_Water",
    "DBATeamId=1",
    "TeamId=2",
    "rat_water",
    "Rat_Fire",
    "TryExtractCharacterBuildSummary",
    "TryExtractTeamId",
    "TamperedSummary",
    "MissingSummary",
    "MissingTeamId",
    "NonPositiveTeamId"
)

Write-Host "PASS: Dedicated Server URL build-summary admission contract" -ForegroundColor Green
