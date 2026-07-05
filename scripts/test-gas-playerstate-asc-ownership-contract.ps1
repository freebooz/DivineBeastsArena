<#
Validates the GAS PlayerState ownership contract.

Goals:
- ADBAPlayerState owns the ASC and attribute sets for multiplayer MOBA gameplay.
- ADBAZodiacCharacterBase is the AvatarActor and initializes AbilityActorInfo from
  server PossessedBy and client OnRep_PlayerState.
- Character compatibility APIs prefer ADBAPlayerState when resolving DBA ASC.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

$playerStateHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Player\DBAPlayerState.h"
$playerStateCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Player\DBAPlayerState.cpp"
$characterHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h"
$characterCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

$playerStateHeader = Get-Content -LiteralPath $playerStateHeaderPath -Encoding UTF8 -Raw
$playerStateCpp = Get-Content -LiteralPath $playerStateCppPath -Encoding UTF8 -Raw
$characterHeader = Get-Content -LiteralPath $characterHeaderPath -Encoding UTF8 -Raw
$characterCpp = Get-Content -LiteralPath $characterCppPath -Encoding UTF8 -Raw

function Assert-True {
  param(
    [Parameter(Mandatory = $true)][bool]$Condition,
    [Parameter(Mandatory = $true)][string]$Message
  )

  if (-not $Condition) {
    throw $Message
  }
}

Assert-True ($playerStateHeader.Contains('#include "AbilitySystemInterface.h"')) `
  "Expected ADBAPlayerState to include AbilitySystemInterface.h."
Assert-True ($playerStateHeader -match "class\s+DIVINEBEASTSARENA_API\s+ADBAPlayerState\s*:\s*public\s+APlayerState\s*,\s*public\s+IAbilitySystemInterface") `
  "Expected ADBAPlayerState to publicly inherit IAbilitySystemInterface."
Assert-True ($playerStateHeader.Contains("class UDBAAbilitySystemComponent;")) `
  "Expected ADBAPlayerState.h to forward declare UDBAAbilitySystemComponent."
Assert-True ($playerStateHeader.Contains("class UDBABattleAttributeSet;")) `
  "Expected ADBAPlayerState.h to forward declare UDBABattleAttributeSet."
Assert-True ($playerStateHeader.Contains("class UDBAHeroGrowthAttributeSet;")) `
  "Expected ADBAPlayerState.h to forward declare UDBAHeroGrowthAttributeSet."
Assert-True ($playerStateHeader -match "virtual\s+UAbilitySystemComponent\*\s+GetAbilitySystemComponent\(\)\s+const\s+override\s*;") `
  "Expected ADBAPlayerState to override GetAbilitySystemComponent."
Assert-True ($playerStateHeader -match "UDBAAbilitySystemComponent\*\s+GetDBAAbilitySystemComponent\(\)\s+const") `
  "Expected ADBAPlayerState to expose typed GetDBAAbilitySystemComponent."
Assert-True ($playerStateHeader -match "TObjectPtr<\s*UDBAAbilitySystemComponent\s*>\s+AbilitySystemComponent") `
  "Expected ADBAPlayerState to own UDBAAbilitySystemComponent."
Assert-True ($playerStateHeader -match "TObjectPtr<\s*UDBABattleAttributeSet\s*>\s+BattleAttributeSet") `
  "Expected ADBAPlayerState to own UDBABattleAttributeSet."
Assert-True ($playerStateHeader -match "TObjectPtr<\s*UDBAHeroGrowthAttributeSet\s*>\s+HeroGrowthAttributeSet") `
  "Expected ADBAPlayerState to own UDBAHeroGrowthAttributeSet."

Assert-True ($playerStateCpp.Contains('CreateDefaultSubobject<UDBAAbilitySystemComponent>(TEXT("AbilitySystemComponent"))')) `
  "Expected ADBAPlayerState constructor to create UDBAAbilitySystemComponent."
Assert-True ($playerStateCpp.Contains('CreateDefaultSubobject<UDBABattleAttributeSet>(TEXT("BattleAttributeSet"))')) `
  "Expected ADBAPlayerState constructor to create UDBABattleAttributeSet."
Assert-True ($playerStateCpp.Contains('CreateDefaultSubobject<UDBAHeroGrowthAttributeSet>(TEXT("HeroGrowthAttributeSet"))')) `
  "Expected ADBAPlayerState constructor to create UDBAHeroGrowthAttributeSet."
Assert-True ($playerStateCpp.Contains("SetReplicationMode(EGameplayEffectReplicationMode::Mixed)")) `
  "Expected ADBAPlayerState to set ASC replication mode to Mixed."
Assert-True ($playerStateCpp -match "UAbilitySystemComponent\*\s+ADBAPlayerState::GetAbilitySystemComponent\(\)\s+const") `
  "Expected ADBAPlayerState.cpp to implement GetAbilitySystemComponent."
Assert-True ($playerStateCpp -match "UDBAAbilitySystemComponent\*\s+ADBAPlayerState::GetDBAAbilitySystemComponent\(\)\s+const") `
  "Expected ADBAPlayerState.cpp to implement GetDBAAbilitySystemComponent."

Assert-True ($characterHeader.Contains("virtual void PossessedBy(AController* NewController) override;")) `
  "Expected ADBAZodiacCharacterBase to initialize ASC from server PossessedBy."
Assert-True ($characterHeader.Contains("virtual void OnRep_PlayerState() override;")) `
  "Expected ADBAZodiacCharacterBase to initialize ASC from client OnRep_PlayerState."
Assert-True ($characterHeader.Contains("void InitializeDBAAbilityActorInfo();")) `
  "Expected ADBAZodiacCharacterBase to declare InitializeDBAAbilityActorInfo."

Assert-True ($characterCpp -match "void\s+ADBAZodiacCharacterBase::PossessedBy\s*\(") `
  "Expected ADBAZodiacCharacterBase.cpp to implement PossessedBy."
Assert-True ($characterCpp -match "void\s+ADBAZodiacCharacterBase::OnRep_PlayerState\s*\(") `
  "Expected ADBAZodiacCharacterBase.cpp to implement OnRep_PlayerState."
Assert-True ($characterCpp -match "void\s+ADBAZodiacCharacterBase::InitializeDBAAbilityActorInfo\s*\(") `
  "Expected ADBAZodiacCharacterBase.cpp to implement InitializeDBAAbilityActorInfo."
Assert-True ($characterCpp.Contains("GetPlayerState<ADBAPlayerState>()")) `
  "Expected ADBAZodiacCharacterBase to prefer ADBAPlayerState ASC."
Assert-True ($characterCpp.Contains("InitializeAbilities(DBAPlayerState, this)")) `
  "Expected InitializeDBAAbilityActorInfo to use PlayerState as OwnerActor and Character as AvatarActor."

Write-Host "PASS: GAS PlayerState ASC ownership and avatar initialization contract" -ForegroundColor Green
