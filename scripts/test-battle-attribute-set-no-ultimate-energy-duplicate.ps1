<#
Validates that UltimateEnergy has a single authoritative runtime source.

Current GAS architecture:
- UDBAAbilitySystemComponent owns replicated UltimateEnergy and its delegate.
- UDBABattleAttributeSet owns normal combat attributes only.
- BattleAttributeSet must not define, replicate, clamp, or expose a duplicate
  UltimateEnergy GameplayAttribute.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

$battleHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Attributes\DBABattleAttributeSet.h"
$battleCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Attributes\DBABattleAttributeSet.cpp"
$ascHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySystemComponent.h"
$ascCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"

$battleHeader = Get-Content -LiteralPath $battleHeaderPath -Encoding UTF8 -Raw
$battleCpp = Get-Content -LiteralPath $battleCppPath -Encoding UTF8 -Raw
$ascHeader = Get-Content -LiteralPath $ascHeaderPath -Encoding UTF8 -Raw
$ascCpp = Get-Content -LiteralPath $ascCppPath -Encoding UTF8 -Raw

function Assert-True {
  param(
    [Parameter(Mandatory = $true)][bool]$Condition,
    [Parameter(Mandatory = $true)][string]$Message
  )

  if (-not $Condition) {
    throw $Message
  }
}

Assert-True (-not ($battleHeader -match "FGameplayAttributeData\s+UltimateEnergy")) `
  "BattleAttributeSet must not declare duplicate UltimateEnergy."
Assert-True (-not $battleHeader.Contains("BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, UltimateEnergy)")) `
  "BattleAttributeSet must not expose UltimateEnergy attribute accessors."
Assert-True (-not $battleHeader.Contains("OnRep_UltimateEnergy")) `
  "BattleAttributeSet must not declare UltimateEnergy OnRep."

Assert-True (-not $battleCpp.Contains("InitUltimateEnergy(")) `
  "BattleAttributeSet constructor must not initialize duplicate UltimateEnergy."
Assert-True (-not $battleCpp.Contains("DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, UltimateEnergy")) `
  "BattleAttributeSet must not replicate duplicate UltimateEnergy."
Assert-True (-not $battleCpp.Contains("GetUltimateEnergyAttribute()")) `
  "BattleAttributeSet must not clamp duplicate UltimateEnergy attribute."
Assert-True (-not $battleCpp.Contains("SetUltimateEnergy(")) `
  "BattleAttributeSet must not write duplicate UltimateEnergy."
Assert-True (-not $battleCpp.Contains("UDBABattleAttributeSet::OnRep_UltimateEnergy")) `
  "BattleAttributeSet must not implement UltimateEnergy OnRep."

Assert-True ($ascHeader -match "ReplicatedUsing\s*=\s*OnRep_UltimateEnergy") `
  "ASC must own replicated UltimateEnergy."
Assert-True ($ascHeader.Contains("FOnUltimateEnergyChanged OnUltimateEnergyChanged")) `
  "ASC must expose UltimateEnergy change delegate."
Assert-True ($ascHeader.Contains("float GetUltimateEnergy() const")) `
  "ASC must expose typed UltimateEnergy getter."
Assert-True ($ascCpp.Contains("DOREPLIFETIME(UDBAAbilitySystemComponent, UltimateEnergy)")) `
  "ASC must replicate UltimateEnergy."
Assert-True ($ascCpp.Contains("BroadcastUltimateEnergyChanged()")) `
  "ASC must centralize UltimateEnergy change broadcasts."

Write-Host "PASS: BattleAttributeSet has no duplicate UltimateEnergy runtime source" -ForegroundColor Green
