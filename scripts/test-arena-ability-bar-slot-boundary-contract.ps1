<#
Validates Arena AbilityBar public entry points reject invalid 1-based skill slots
before forwarding updates to Blueprint events.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$abilityBarCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAAbilityBarWidgetBase.cpp"

function Assert-True {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

$abilityBarCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $abilityBarCppPath

Assert-True ($abilityBarCpp.Contains('#include "GameDBA/Core/DBAConstants.h"')) "Expected AbilityBar implementation to include DBAConstants for the skill slot boundary."
Assert-True ($abilityBarCpp -match "SlotIndex\s*<\s*1\s*\|\|\s*SlotIndex\s*>\s*DBAConstants::CoreCombatInputCount") "Expected AbilityBar to guard against invalid 1-based skill slots."
Assert-True ($abilityBarCpp -match "UpdateAbility[\s\S]*SlotIndex\s*<\s*1\s*\|\|\s*SlotIndex\s*>\s*DBAConstants::CoreCombatInputCount[\s\S]*return;[\s\S]*BP_OnAbilityUpdated") "Expected UpdateAbility to return before BP_OnAbilityUpdated for invalid slots."
Assert-True ($abilityBarCpp -match "SetAbilityEnabled[\s\S]*SlotIndex\s*<\s*1\s*\|\|\s*SlotIndex\s*>\s*DBAConstants::CoreCombatInputCount[\s\S]*return;[\s\S]*BP_OnAbilityEnabledChanged") "Expected SetAbilityEnabled to return before BP_OnAbilityEnabledChanged for invalid slots."

Write-Host "PASS: Arena AbilityBar slot boundary contract" -ForegroundColor Green
