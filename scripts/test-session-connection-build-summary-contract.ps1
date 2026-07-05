<#
Validates the session connection build summary handoff contract.

This guard keeps the backend-frozen Zodiac / Element / FiveCamp /
FixedSkillGroupId path visible from SessionConnectionResponse through the
Unreal travel URL options used by Dedicated Server admission.
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
        throw "$RelativePath is missing session connection build summary contract symbols: $($missing -join ', ')"
    }
}

Assert-FileContains "DBA_GameBackend\Game.Shared\Contracts\Session\SessionDtos.cs" @(
    "SessionConnectionResponse",
    "int TeamId",
    "CharacterBuildSummaryDto? CharacterBuildSummary"
)

Assert-FileContains "DBA_GameBackend\Game.Api\Services\Session\SessionService.cs" @(
    "GetConnectionInfoAsync",
    'session.Status is not ("WAITING_PLAYERS" or "IN_PROGRESS")',
    "playerSession.Team",
    "EnsureFrozenBuildSummaryAsync",
    "ToCharacterBuildSummary(playerSession)",
    "CharacterBuildRules.BuildFixedSkillGroupId",
    "CharacterBuildRules.BuildSummary",
    "playerSession.Zodiac = existingBuildSummary.Zodiac",
    "playerSession.PrimaryElement = existingBuildSummary.PrimaryElement",
    "playerSession.FiveCamp = existingBuildSummary.FiveCamp",
    "playerSession.FixedSkillGroupId = existingBuildSummary.FixedSkillGroupId"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\RoomSessionServiceTests.cs" @(
    "GetConnectionInfoAsync_WhenServerIsAllocatedButNotReady_ReturnsNullWithoutReissuingToken",
    'storedSession.Status = "ALLOCATING_SERVER"',
    'storedSession.Status = "WAITING_PLAYERS"',
    "originalTokenHash",
    "originalTokenExpiresAt",
    "GetConnectionInfoAsync_ReturnsFrozenSelectedCharacterBuildSummary",
    "Assert.Equal(1, connection.TeamId)",
    "GetConnectionInfoAsync_WhenSelectedCharacterBuildSummaryIsPadded_ReturnsNormalizedSummary",
    "GetConnectionInfoAsync_WhenExistingFrozenBuildSummaryIsPadded_PersistsNormalizedSummary",
    "GetConnectionInfoAsync_WhenExistingFrozenFixedSkillGroupIsTampered_ReturnsNull",
    "GetConnectionInfoAsync_WhenExistingFrozenBuildSummaryIsPartial_ReturnsNull",
    "GetConnectionInfoAsync_WhenSelectedCharacterFixedSkillGroupIsTampered_FreezesComputedSkillGroup",
    'Assert.Equal("Rat_Water", connection!.CharacterBuildSummary!.FixedSkillGroupId)'
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendSessionService.cpp" @(
    "TryBuildTravelUrlFromConnectionData",
    "characterBuildSummary",
    "Connection.TeamId",
    'TEXT("DBATeamId")',
    'TEXT("teamId")',
    'TEXT("DBAZodiac")',
    'TEXT("DBAElement")',
    'TEXT("DBAFiveCamp")',
    'TEXT("DBAFixedSkillGroupId")',
    'TEXT("zodiac")',
    'TEXT("primaryElement")',
    'TEXT("fiveCamp")',
    'TEXT("fixedSkillGroupId")'
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\Tests\GameBackendSessionServiceTests.cpp" @(
    "BuildTravelUrlIncludesFrozenBuildSummary",
    "DBATeamId=1",
    "ConnectionJsonBuildsTravelUrlWithNestedBuildSummary",
    "DBATeamId=2",
    "ConnectionJsonAcceptsNestedServerAliases",
    "ConnectionJsonAcceptsResponseEnvelopeData",
    "DBAZodiac=Rat",
    "DBAElement=Water",
    "DBAFiveCamp=East",
    "DBAFixedSkillGroupId=Rat_Water",
    "DBAFixedSkillGroupId=Tiger_Fire",
    "DBAFixedSkillGroupId=Dragon_Wood",
    "DBAFixedSkillGroupId=Snake_Gold"
)

Write-Host "PASS: session connection build summary contract" -ForegroundColor Green
