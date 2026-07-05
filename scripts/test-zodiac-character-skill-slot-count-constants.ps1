<#
Validates Zodiac character cooldown arrays use DBAConstants for playable skill
slot counts instead of hard-coded 7 / 5 literals.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$constantsPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Core\DBAConstants.h"
$characterCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$constants = Get-Content -Raw -Encoding UTF8 -LiteralPath $constantsPath
$characterCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterCppPath

Assert-True ($constants -match "constexpr\s+int32\s+ArenaCombatSkillSlotCount\s*=\s*DBAConstants::ActiveSkillCount\s*\+\s*1\s*;") `
    "Expected DBAConstants::ArenaCombatSkillSlotCount to define Skill01~04 plus Ultimate."
Assert-True ($constants -match "constexpr\s+int32\s+PlayableSkillSlotCount\s*=\s*DBAConstants::CoreCombatInputCount\s*;") `
    "Expected DBAConstants::PlayableSkillSlotCount to define the visible playable skill slots."
Assert-True ($constants -match "constexpr\s+int32\s+PlayableSkillArraySize\s*=\s*DBAConstants::PlayableSkillSlotCount\s*\+\s*1\s*;") `
    "Expected DBAConstants::PlayableSkillArraySize to account for 1-based skill slot indexing."

Assert-True ($characterCpp.Contains("DBAConstants::PlayableSkillArraySize")) `
    "Expected Zodiac character cooldown arrays to use DBAConstants::PlayableSkillArraySize."

Assert-True (-not ($characterCpp -match "SkillCooldowns\.Init\(0\.0f,\s*7\)")) `
    "Zodiac character still initializes SkillCooldowns with hard-coded 7."
Assert-True (-not ($characterCpp -match "SkillMaxCooldowns\.Init\(0\.0f,\s*7\)")) `
    "Zodiac character still initializes SkillMaxCooldowns with hard-coded 7."
Assert-True (-not ($characterCpp -match "SkillCooldowns\.SetNumZeroed\(7\)")) `
    "Zodiac character still resizes SkillCooldowns with hard-coded 7."
Assert-True (-not ($characterCpp -match "SkillMaxCooldowns\.SetNumZeroed\(7\)")) `
    "Zodiac character still resizes SkillMaxCooldowns with hard-coded 7."
Assert-True (-not ($characterCpp -match "SkillSlot\s*<=\s*5")) `
    "Zodiac character still loops combat skill slots with hard-coded <= 5."
Assert-True (-not ($characterCpp -match "for\s*\(\s*int32\s+SkillSlot\s*=\s*1\s*;\s*SkillSlot\s*<=\s*DBAConstants::ArenaCombatSkillSlotCount")) `
    "Zodiac character should not reintroduce default combat-slot loops; playable skill specs must come from DataAsset."
Assert-True (-not $characterCpp.Contains("GetDefaultLobbySkillSpec")) `
    "Zodiac character should not reintroduce C++ default lobby skill specs."

Write-Host "PASS: Zodiac character skill slot count constants contract" -ForegroundColor Green
