<#
Validates Arena AbilityBar uses FixedSkillGroup runtime display config from GAS
and loads skill icons asynchronously instead of hardcoding UI display data.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$skillTypesPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBAPlayableSkillTypes.h"
$abilityBarHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Arena\UDBAAbilityBarWidgetBase.h"
$abilityBarCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\UDBAAbilityBarWidgetBase.cpp"
$slotCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Arena\AbilityBar\DBAAbilitySlotWidget.cpp"

function Assert-True {
  param(
    [Parameter(Mandatory = $true)][bool]$Condition,
    [Parameter(Mandatory = $true)][string]$Message
  )

  if (-not $Condition) {
    throw $Message
  }
}

$skillTypes = Get-Content -LiteralPath $skillTypesPath -Encoding UTF8 -Raw
$abilityBarHeader = Get-Content -LiteralPath $abilityBarHeaderPath -Encoding UTF8 -Raw
$abilityBarCpp = Get-Content -LiteralPath $abilityBarCppPath -Encoding UTF8 -Raw
$slotCpp = Get-Content -LiteralPath $slotCppPath -Encoding UTF8 -Raw

Assert-True ($skillTypes -match "class\s+UTexture2D\s*;") `
  "Expected playable skill spec header to forward declare UTexture2D."
Assert-True ($skillTypes -match "TSoftObjectPtr<\s*UTexture2D\s*>\s+Icon") `
  "Expected playable skill runtime spec to carry a soft icon reference."

Assert-True ($abilityBarHeader -match "ApplyRuntimeConfigToSkillSpec\s*\(\s*FDBAPlayableSkillRuntimeSpec&\s+InOutSkillSpec\s*\)\s+const") `
  "Expected AbilityBar to declare runtime config display overlay helper."
Assert-True ($abilityBarHeader -match "MapSkillSlotToAbilityInputID\s*\(\s*int32\s+SkillSlot\s*\)\s+const") `
  "Expected AbilityBar to map UI skill slot to GAS InputID."
Assert-True ($abilityBarHeader -match "RequestSkillIconAsync\s*\(\s*UDBAAbilitySlotWidget\*\s+SlotWidget\s*,\s*const\s+FDBAPlayableSkillRuntimeSpec&\s+SkillSpec\s*\)") `
  "Expected AbilityBar to declare asynchronous icon request helper."

Assert-True ($abilityBarCpp -match "GameDBA/GAS/DBAAbilitySystemComponent\.h") `
  "Expected AbilityBar to include DBA ASC for runtime config lookup."
Assert-True ($abilityBarCpp -match "GameDBA/Utilities/DBAAsyncAssetLoader\.h") `
  "Expected AbilityBar to include async asset loader for icons."
Assert-True ($abilityBarCpp -match "FindAbilityRuntimeConfigByInputID\s*\(\s*MapSkillSlotToAbilityInputID") `
  "Expected AbilityBar to look up runtime config by mapped GAS InputID."
Assert-True ($abilityBarCpp -match "RuntimeConfig->DisplayName") `
  "Expected AbilityBar to apply DataAsset display name."
Assert-True ($abilityBarCpp -match "RuntimeConfig->Icon") `
  "Expected AbilityBar to apply DataAsset icon."
Assert-True ($abilityBarCpp -match "RuntimeConfig->CooldownDuration") `
  "Expected AbilityBar to apply DataAsset cooldown duration for UI totals."
Assert-True ($abilityBarCpp -match "RequestSkillIconAsync\s*\(\s*SlotWidget\s*,\s*SkillSpec\s*\)") `
  "Expected AbilityBar refresh path to request skill icons asynchronously."
Assert-True ($abilityBarCpp -match "DBAAsyncAssetLoader::RequestAsyncAsset<\s*UTexture2D\s*>") `
  "Expected AbilityBar to async-load UTexture2D icons."
Assert-True ($abilityBarCpp -notmatch "LoadSynchronous\s*\(") `
  "Expected AbilityBar not to synchronously load icon assets."

Assert-True ($slotCpp -match "Info\.Icon\s*=\s*SkillSpec\.Icon\.Get\s*\(\s*\)") `
  "Expected AbilitySlot to consume already-loaded skill icon references."

$successMessage = [string]::Concat([char[]](36890, 36807, 65306, 25216, 33021, 26639, 23637, 31034, 25968, 25454, 24050, 25509, 20837, 22266, 23450, 25216, 33021, 32452, 36816, 34892, 37197, 32622))
Write-Host $successMessage -ForegroundColor Green
