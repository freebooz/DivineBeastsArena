<#
Validates that AbilitySystem cooldown synchronization is driven by GAS
ActiveGameplayEffect add/remove events instead of a polling timer in ASC.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

function Assert-True {
  param(
    [bool]$Condition,
    [string]$Message
  )

  if (-not $Condition) {
    throw $Message
  }
}

function Get-FileContent {
  param([string]$RelativePath)

  $path = Join-Path -Path $repoRoot -ChildPath $RelativePath
  if (-not (Test-Path -LiteralPath $path)) {
    throw "Required file is missing: $RelativePath"
  }

  return Get-Content -LiteralPath $path -Encoding UTF8 -Raw
}

$ascHeader = Get-FileContent "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySystemComponent.h"
$ascCpp = Get-FileContent "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"

Assert-True ($ascHeader -match "HandleCooldownGameplayEffectAddedToSelf\s*\(") `
  "AbilitySystemComponent must declare an ActiveGameplayEffect added handler for cooldown sync."

Assert-True ($ascHeader -match "HandleCooldownGameplayEffectRemoved\s*\(") `
  "AbilitySystemComponent must declare an ActiveGameplayEffect removed handler for cooldown sync."

Assert-True ($ascCpp -match "OnActiveGameplayEffectAddedDelegateToSelf\.AddUObject\s*\(\s*this\s*,\s*&UDBAAbilitySystemComponent::HandleCooldownGameplayEffectAddedToSelf\s*\)") `
  "AbilitySystemComponent must bind cooldown sync to ActiveGameplayEffect added events."

Assert-True ($ascCpp -match "OnAnyGameplayEffectRemovedDelegate\s*\(\s*\)\.AddUObject\s*\(\s*this\s*,\s*&UDBAAbilitySystemComponent::HandleCooldownGameplayEffectRemoved\s*\)") `
  "AbilitySystemComponent must bind cooldown sync to ActiveGameplayEffect removed events."

Assert-True ($ascCpp -match "OnActiveGameplayEffectAddedDelegateToSelf\.RemoveAll\s*\(\s*this\s*\)") `
  "AbilitySystemComponent must unbind ActiveGameplayEffect added events in EndPlay."

Assert-True ($ascCpp -match "OnAnyGameplayEffectRemovedDelegate\s*\(\s*\)\.RemoveAll\s*\(\s*this\s*\)") `
  "AbilitySystemComponent must unbind ActiveGameplayEffect removed events in EndPlay."

Assert-True ($ascCpp -match "void\s+UDBAAbilitySystemComponent::HandleCooldownGameplayEffectAddedToSelf[\s\S]*SyncCooldownsToCharacter\s*\(") `
  "Added-effect cooldown handler must synchronize cooldowns to the character mirror."

Assert-True ($ascCpp -match "void\s+UDBAAbilitySystemComponent::HandleCooldownGameplayEffectRemoved[\s\S]*SyncCooldownsToCharacter\s*\(") `
  "Removed-effect cooldown handler must synchronize cooldowns to the character mirror."

Assert-True ($ascCpp -notmatch "SetTimer\s*\(\s*CooldownSyncTimerHandle") `
  "AbilitySystemComponent must not poll cooldown sync through CooldownSyncTimerHandle."

Assert-True ($ascCpp -notmatch "ClearTimer\s*\(\s*CooldownSyncTimerHandle") `
  "AbilitySystemComponent must not clear a removed cooldown polling timer."

Assert-True ($ascHeader -notmatch "CooldownSyncInterval") `
  "AbilitySystemComponent must not keep a polling cooldown sync interval."

Write-Host "PASS: AbilitySystem cooldown event-driven sync contract" -ForegroundColor Green
