<#
Validates the Dedicated Server Runtime player-joined build summary contract.

This lightweight guard keeps the backend-authoritative Zodiac / Element /
FixedSkillGroupId rules visible in production evidence automation without
needing to run Unreal Editor.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath

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
        throw "$RelativePath is missing runtime player-joined build summary contract symbols: $($missing -join ', ')"
    }
}

Assert-FileContains "DBA_GameBackend\Game.Shared\Contracts\GameServer\GameServerDtos.cs" @(
    "RuntimePlayerJoinedRequest",
    "string? Zodiac",
    "string? PrimaryElement",
    "string? FiveCamp",
    "string? FixedSkillGroupId"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Services\Runtime\RuntimePlayerJoinValidator.cs" @(
    "RuntimePlayerJoinValidator",
    "BuildSummaryMismatchMessage",
    "BuildSummaryMissingMessage",
    "FrozenBuildSummaryInvalidMessage",
    "CharacterBuildRules.BuildFixedSkillGroupId",
    "BuildPlayerJoinedEventPayload",
    "team = NormalizeEventValue(playerSession.Team)",
    "fiveCamp = NormalizeEventValue(playerSession.FiveCamp)",
    "fixedSkillGroupId = NormalizeEventValue(playerSession.FixedSkillGroupId)"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\RuntimePlayerJoinBuildSummaryTests.cs" @(
    "ValidateBuildSummary_WhenFixedSkillGroupIsTampered_ReturnsFalse",
    "ValidateBuildSummary_WhenFiveCampChangesButSkillGroupMatches_ReturnsTrue",
    "ValidateBuildSummary_WhenFrozenFixedSkillGroupDoesNotMatchFrozenIdentity_ReturnsFalse",
    "ValidateBuildSummary_WhenNoFrozenBuildSummaryExists_AllowsLegacySession",
    "ValidateBuildSummary_WhenFrozenBuildSummaryIsPartial_ReturnsFalse",
    "ValidateBuildSummary_WhenFrozenBuildExistsButFixedSkillGroupIsMissing_ReturnsFalse",
    "ValidateBuildSummary_WhenFrozenBuildExistsButRequiredChoiceIsNone_ReturnsFalse",
    "ValidateBuildSummary_WhenFrozenBuildContainsNone_ReturnsFalse",
    "BuildPlayerJoinedEventPayload_WhenFiveCampRequestDiffers_UsesFrozenBuildSummary",
    "BuildPlayerJoinedEventPayload_WhenFrozenBuildSummaryIsPadded_WritesNormalizedSummary",
    "BuildPlayerJoinedEventPayload_WhenTeamIsPadded_WritesNormalizedTeam",
    "BuildPlayerJoinedEventPayload_WhenNoFrozenBuildSummaryExists_DoesNotInventDefaultSummary"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendRuntimeService.cpp" @(
    'TEXT("team")',
    'TEXT("zodiac")',
    'TEXT("primaryElement")',
    'TEXT("fiveCamp")',
    'TEXT("fixedSkillGroupId")'
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Framework\DBAGameModeBase.cpp" @(
    "BuildBackendRuntimeTeamName",
    "ResolveBackendMatchTeamId(PlayerController, BackendRuntimePlayerTeamIds)",
    "RuntimeService->NotifyPlayerJoined(PlayerId, PlayerSessionToken, BackendRuntimeTeam"
)

Write-Host "PASS: runtime player-joined build summary contract" -ForegroundColor Green
