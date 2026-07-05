<#
Validates Arena AbilityBar maps 1-based skill slots onto the 0-based cooldown array from GAS.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).ProviderPath
$abilityBarCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAAbilityBarWidgetBase.cpp"
$abilitySlotHeaderPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\AbilityBar\DBAAbilitySlotWidget.h"
$abilitySlotCppPath = Join-Path $repoRoot "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\AbilityBar\DBAAbilitySlotWidget.cpp"

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
$abilitySlotHeader = Get-Content -Raw -Encoding UTF8 -LiteralPath $abilitySlotHeaderPath
$abilitySlotCpp = Get-Content -Raw -Encoding UTF8 -LiteralPath $abilitySlotCppPath

Assert-True ($abilityBarCpp -match "RefreshCooldowns[\s\S]*const\s+int32\s+CooldownArrayIndex\s*=\s*SkillSlot\s*-\s*1") "Expected RefreshCooldowns to convert 1-based SkillSlot to 0-based CooldownArrayIndex."
Assert-True ($abilityBarCpp -match "RefreshCooldowns[\s\S]*Cooldowns\.IsValidIndex\(CooldownArrayIndex\)\s*\?\s*Cooldowns\[CooldownArrayIndex\]") "Expected RefreshCooldowns to read cooldowns by CooldownArrayIndex."
Assert-True ($abilityBarCpp -match "RefreshCooldowns[\s\S]*UpdateAbility\(SkillSlot,\s*Remaining,\s*0\.0f\)") "Expected RefreshCooldowns to keep broadcasting the original 1-based SkillSlot to widgets."
Assert-True ($abilityBarCpp -notmatch "RefreshCooldowns[\s\S]{0,520}Cooldowns\.IsValidIndex\(SkillSlot\)\s*\?\s*Cooldowns\[SkillSlot\]") "Expected RefreshCooldowns to avoid indexing cooldowns directly by 1-based SkillSlot."
Assert-True ($abilityBarCpp -match "UpdateAbility[\s\S]*ClampedCooldown\s*=\s*FMath::Max\(0\.0f,\s*Cooldown\)[\s\S]*bOnCooldown\s*=\s*ClampedCooldown\s*>\s*0\.0f") "Expected UpdateAbility to normalize negative cooldowns before on-cooldown state."
Assert-True ($abilityBarCpp -match "UpdateAbility[\s\S]*ClampedManaCost\s*=\s*FMath::Max\(0\.0f,\s*ManaCost\)[\s\S]*BP_OnAbilityUpdated\(SlotIndex,\s*ClampedCooldown,\s*ClampedManaCost,\s*bOnCooldown\)") "Expected UpdateAbility to broadcast non-negative cooldown and mana cost."
Assert-True ($abilityBarCpp -match "SlotWidget->SetCooldown\(ClampedCooldown,\s*TotalCooldown\)") "Expected AbilityBar slot widget updates to receive normalized cooldown."
Assert-True ($abilitySlotHeader -match "virtual\s+void\s+NativeConstruct\(\)\s+override") "Expected AbilitySlot to replay cached state after widget tree binding."
Assert-True ($abilitySlotCpp -match "SetAbilityInfo[\s\S]*AbilityInfo\.Cooldown\s*=\s*FMath::Max\(0\.0f,\s*AbilityInfo\.Cooldown\)[\s\S]*AbilityInfo\.CurrentCooldown\s*=\s*FMath::Clamp\(AbilityInfo\.CurrentCooldown,\s*0\.0f,\s*AbilityInfo\.Cooldown\)") "Expected AbilitySlot SetAbilityInfo to normalize cooldown fields before display refresh."
Assert-True ($abilitySlotCpp -match "SetCooldown[\s\S]*AbilityInfo\.Cooldown\s*=\s*FMath::Max\(0\.0f,\s*TotalTime\)[\s\S]*AbilityInfo\.CurrentCooldown\s*=\s*FMath::Clamp\(RemainingTime,\s*0\.0f,\s*AbilityInfo\.Cooldown\)") "Expected AbilitySlot SetCooldown to clamp remaining cooldown against a non-negative total."
Assert-True ($abilitySlotCpp -match "NativeConstruct[\s\S]{0,260}UpdateIconDisplay\(\)[\s\S]{0,260}UpdateHotkeyDisplay\(\)[\s\S]{0,260}SetAvailable\(AbilityInfo\.bEnabled\)[\s\S]{0,260}UpdateCooldownDisplay\(\)") "Expected AbilitySlot NativeConstruct to replay cached icon, hotkey, availability, and cooldown state."
Assert-True ($abilitySlotCpp -match "SetAbilityInfo[\s\S]{0,620}UpdateHotkeyDisplay\(\)") "Expected AbilitySlot SetAbilityInfo to use the same hotkey refresh path as NativeConstruct."
Assert-True ($abilitySlotCpp -match "UpdateIconDisplay[\s\S]{0,360}else[\s\S]{0,160}SkillIcon->SetBrush\(FSlateBrush\(\)\)") "Expected AbilitySlot UpdateIconDisplay to clear stale icons when the cached ability has no icon."
Assert-True ($abilitySlotCpp -match "UpdateCooldownDisplay[\s\S]{0,900}else[\s\S]{0,260}CooldownText->SetText\(FText::GetEmpty\(\)\)") "Expected AbilitySlot UpdateCooldownDisplay to clear stale cooldown text when cooldown ends."

Write-Host "PASS: Arena AbilityBar cooldown slot indexing contract" -ForegroundColor Green
