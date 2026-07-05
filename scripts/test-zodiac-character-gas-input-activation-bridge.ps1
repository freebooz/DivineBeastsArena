<#
Validates Zodiac character equipped-skill casts prefer the server-authoritative
GAS input activation path before falling back to legacy spawned skill actors.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$ascHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySystemComponent.h"
$ascCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"
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

$ascHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $ascHeaderPath
$ascCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $ascCppPath
$characterCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $characterCppPath

Assert-True ($ascHeader -match "bool\s+TryActivateAbilityByInputID\s*\(\s*int32\s+InputID\s*,\s*AActor\*\s+Target\s*\)") "Expected UDBAAbilitySystemComponent to expose TryActivateAbilityByInputID(int32 InputID, AActor* Target)."
Assert-True ($ascCpp -match "bool\s+UDBAAbilitySystemComponent::TryActivateAbilityByInputID\s*\(\s*int32\s+InputID\s*,\s*AActor\*\s+Target\s*\)") "Expected TryActivateAbilityByInputID implementation."
Assert-True ($ascCpp -match "TryActivateAbilityByInputID[\s\S]*GetOwnerRole\(\)\s*!=\s*ROLE_Authority") "Expected GAS input activation bridge to reject non-authority calls."
Assert-True ($ascCpp -match "TryActivateAbilityByInputID[\s\S]*Spec->InputID\s*==\s*InputID") "Expected GAS input activation bridge to find ability specs by InputID."
Assert-True ($ascCpp -match "TryActivateAbilityByInputID[\s\S]*TryActivateAbility\(\s*Spec->Handle\s*,\s*false\s*\)") "Expected GAS input activation bridge to activate the matching ability handle."
Assert-True ($ascCpp -match "TryActivateAbilityByInputID[\s\S]*SyncCooldownsToCharacter\(\)") "Expected successful GAS input activation to sync cooldowns to the character/HUD bridge."

Assert-True ($characterCpp -match "CastEquippedSkillInternal[\s\S]*TryActivateAbilityByInputID\(") "Expected CastEquippedSkillInternal to try GAS input activation before legacy fallback."
Assert-True ($characterCpp -match "MapEquippedSkillSlotToAbilityInputID") "Expected Character skill slot to Ability InputID mapping helper."
Assert-True ($characterCpp -match "MapEquippedSkillSlotToAbilityInputID[\s\S]*EDBAAbilityInputID::Skill01") "Expected SkillSlot 1 to map to Skill01."
Assert-True ($characterCpp -match "MapEquippedSkillSlotToAbilityInputID[\s\S]*EDBAAbilityInputID::Ultimate") "Expected Ultimate equipped skill slot to map to Ultimate."

Write-Host "PASS: Zodiac character GAS input activation bridge contract" -ForegroundColor Green
