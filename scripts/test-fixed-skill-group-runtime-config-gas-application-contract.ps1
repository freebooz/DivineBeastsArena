<#
Validates FixedSkillGroup runtime ability config is applied by GAS instead
of remaining a validation-only DataAsset structure.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$ascHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySystemComponent.h"
$ascCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"
$elementAbilityCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAElementAbilityBase.cpp"

$ascHeader = Get-Content -LiteralPath $ascHeaderPath -Encoding UTF8 -Raw
$ascCpp = Get-Content -LiteralPath $ascCppPath -Encoding UTF8 -Raw
$elementAbilityCpp = Get-Content -LiteralPath $elementAbilityCppPath -Encoding UTF8 -Raw

function Assert-True {
  param(
    [Parameter(Mandatory = $true)][bool]$Condition,
    [Parameter(Mandatory = $true)][string]$Message
  )

  if (-not $Condition) {
    throw $Message
  }
}

Assert-True ($ascHeader -match "GameDBA/GAS/DBAAbilitySetLibrary\.h") `
  "Expected ASC header to include FixedSkillGroup runtime config type."
Assert-True ($ascHeader -match "TMap<int32,\s*FDBAAbilityRuntimeConfig>\s+AbilityRuntimeConfigsByInputID") `
  "Expected ASC to cache runtime ability config by GAS InputID."
Assert-True ($ascHeader -match "FindAbilityRuntimeConfigByInputID\s*\(\s*int32\s+InputID\s*\)\s+const") `
  "Expected ASC to expose C++ runtime config lookup by InputID."

Assert-True ($ascCpp -match "AbilityRuntimeConfigsByInputID\.Empty\s*\(") `
  "Expected ASC to clear runtime config cache when abilities are removed or regranted."
Assert-True ($ascCpp -match "AbilityRuntimeConfigsByInputID\.Add\s*\(\s*InputID\s*,\s*\*RuntimeConfig\s*\)") `
  "Expected grant path to cache DataAsset runtime config for the granted InputID."
Assert-True ($ascCpp -match "Skill01RuntimeConfig[\s\S]*Skill02RuntimeConfig[\s\S]*Skill03RuntimeConfig[\s\S]*Skill04RuntimeConfig") `
  "Expected grant path to pass all four DataAsset runtime configs into ASC."
Assert-True ($ascCpp -match "const\s+FDBAAbilityRuntimeConfig\*\s+UDBAAbilitySystemComponent::FindAbilityRuntimeConfigByInputID") `
  "Expected ASC runtime config lookup implementation."

Assert-True ($elementAbilityCpp -match "GameDBA/GAS/DBAAbilitySystemComponent\.h") `
  "Expected element abilities to read runtime config from DBA ASC."
Assert-True ($elementAbilityCpp -match "FindAbilityRuntimeConfigByInputID\s*\(") `
  "Expected element abilities to look up runtime config by InputID."
Assert-True ($elementAbilityCpp -match "ResolveRuntimeEnergyCost") `
  "Expected element abilities to resolve energy cost from runtime config."
Assert-True ($elementAbilityCpp -match "ResolveRuntimeCooldownDuration") `
  "Expected element abilities to resolve cooldown duration from runtime config."
Assert-True ($elementAbilityCpp -match "ResolveRuntimeCooldownGameplayEffectClass") `
  "Expected element abilities to resolve cooldown GE from runtime config."
Assert-True ($elementAbilityCpp -match "ResolveRuntimeCostGameplayEffectClass") `
  "Expected element abilities to resolve cost GE from runtime config."
Assert-True ($elementAbilityCpp -match "RuntimeConfig->EnergyCost") `
  "Expected element ability cost path to use DataAsset EnergyCost."
Assert-True ($elementAbilityCpp -match "RuntimeConfig->CooldownDuration") `
  "Expected element ability cooldown path to use DataAsset CooldownDuration."
Assert-True ($elementAbilityCpp -match "RuntimeConfig->CooldownTag") `
  "Expected element ability cooldown path to use DataAsset CooldownTag."

$successMessage = [string]::Concat([char[]](36890, 36807, 65306, 22266, 23450, 25216, 33021, 32452, 36816, 34892, 37197, 32622, 24050, 25509, 20837, 71, 65, 83))
Write-Host $successMessage -ForegroundColor Green
