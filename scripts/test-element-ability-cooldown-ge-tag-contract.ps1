<#
Validates that normal element abilities apply queryable GAS cooldowns through C++.
The cooldown duration and tag may be configured by ability/data, but runtime cooldown
application must create a duration GameplayEffect with an owning cooldown tag.
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

$elementAbilityHeader = Get-FileContent "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities\DBAElementAbilityBase.h"
$elementAbilityCpp = Get-FileContent "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\Abilities\DBAElementAbilityBase.cpp"
$ascCpp = Get-FileContent "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"

Assert-True ($elementAbilityHeader -match "FGameplayTag\s+CooldownTag") `
  "Element abilities must expose a configurable cooldown tag for data/BP setup."

Assert-True ($elementAbilityHeader -match "CheckCooldown\s*\(") `
  "Element abilities must override CheckCooldown for tag-based GAS cooldown validation."

Assert-True ($elementAbilityHeader -match "ApplyCooldown\s*\(") `
  "Element abilities must override ApplyCooldown to create a cooldown GameplayEffect."

Assert-True ($elementAbilityHeader -match "GetCooldownTimeRemainingAndDuration\s*\(") `
  "Element abilities must override cooldown remaining query with the ability spec handle."

Assert-True ($elementAbilityHeader -match "ResolveRuntimeCooldownDuration\s*\(") `
  "Element abilities must resolve cooldown duration through runtime DataAsset config."

Assert-True ($elementAbilityCpp -match "GameDBA/GAS/Effects/DBAGE_Cooldown\.h") `
  "Element ability cooldown application must use the project cooldown GameplayEffect class."

Assert-True ($elementAbilityCpp -match "GameDBA/Core/DBAGameplayTags\.h") `
  "Element ability cooldown application must use project native cooldown GameplayTags."

Assert-True ($elementAbilityCpp -match "Cooldown_Skill01[\s\S]*Cooldown_Skill02[\s\S]*Cooldown_Skill03[\s\S]*Cooldown_Skill04[\s\S]*Cooldown_Ultimate") `
  "Element ability cooldown resolution must map skill input slots to native cooldown tags."

Assert-True ($elementAbilityCpp -match "ResolveRuntimeCooldownGameplayEffectClass[\s\S]*UDBAGE_Cooldown::StaticClass\s*\(\s*\)") `
  "Element ability cooldown application must resolve a DataAsset cooldown GE and keep UDBAGE_Cooldown as fallback."

Assert-True ($elementAbilityCpp -match "SetDuration\s*\(\s*RuntimeCooldownDuration\s*,\s*true\s*\)") `
  "Element ability cooldown spec must use the runtime-resolved CooldownDuration."

Assert-True ($elementAbilityCpp -match "DynamicGrantedTags\.AddTag\s*\(") `
  "Element ability cooldown spec must add the resolved cooldown tag as a dynamic granted tag."

Assert-True ($elementAbilityCpp -match "FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags") `
  "Element ability cooldown query must use owning cooldown tags."

Assert-True ($elementAbilityCpp -match "ApplyGameplayEffectSpecToOwner\s*\(") `
  "Element ability cooldown application must apply the generated cooldown spec to the owner."

Assert-True ($ascCpp -match "GetCooldownTimeRemainingAndDuration\s*\(\s*Spec->Handle\s*,\s*AbilityActorInfo\.Get\(\)") `
  "AbilitySystem cooldown array sync must query cooldown with the spec handle."

Assert-True ($ascCpp -notmatch "GetCooldownTimeRemaining\s*\(\s*AbilityActorInfo\.Get\(\)\s*\)") `
  "AbilitySystem cooldown array sync must not use the handle-less cooldown query."

$successMessage = [string]::Concat([char[]](36890, 36807, 65306, 20803, 32032, 25216, 33021, 20919, 21364, 71, 69, 26631, 31614, 22865, 32422))
Write-Host $successMessage -ForegroundColor Green
