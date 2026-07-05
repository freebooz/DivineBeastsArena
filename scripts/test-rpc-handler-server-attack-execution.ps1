<#
Validates ServerRequestAttack applies authoritative server-side damage before
reporting a hit confirmation to the owning client.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp"
$cpp = Get-Content -LiteralPath $cppPath -Encoding UTF8 -Raw

function Assert-True {
  param(
    [Parameter(Mandatory = $true)][bool]$Condition,
    [Parameter(Mandatory = $true)][string]$Message
  )

  if (-not $Condition) {
    throw $Message
  }
}

function Get-FunctionBody {
  param(
    [Parameter(Mandatory = $true)][string]$StartPattern,
    [Parameter(Mandatory = $true)][string]$EndPattern,
    [Parameter(Mandatory = $true)][string]$FunctionName
  )

  $match = [regex]::Match($cpp, $StartPattern)
  Assert-True $match.Success "Expected $FunctionName implementation."

  $remaining = $cpp.Substring($match.Index + $match.Length)
  $nextMatch = [regex]::Match($remaining, $EndPattern)
  Assert-True $nextMatch.Success "Expected end boundary after $FunctionName implementation."

  return $cpp.Substring($match.Index, $match.Length + $nextMatch.Index)
}

Assert-True ($cpp.Contains('#include "GameDBA/Combat/DBADamageCalculator.h"')) `
  "Expected RPC handler to include DBADamageCalculator."

$requestAttackImpl = Get-FunctionBody `
  "void\s+ADBARpcHandler::ServerRequestAttack_Implementation\s*\(\s*\)\s*\{" `
  "`nbool\s+ADBARpcHandler::ServerRequestAttack_Validate" `
  "ServerRequestAttack_Implementation"

$requestAttackValidate = Get-FunctionBody `
  "bool\s+ADBARpcHandler::ServerRequestAttack_Validate\s*\(\s*\)\s*\{" `
  "`nvoid\s+ADBARpcHandler::ServerUltimateAbility_Implementation" `
  "ServerRequestAttack_Validate"

foreach ($required in @(
    "ValidateServerCharacterContext(",
    "AActor* Attacker = GetOwner()",
    "AActor* AttackTarget = FindAttackTarget()",
    "if (!AttackTarget)",
    "ClientHitRejected_Implementation(FGameplayAbilitySpecHandle())",
    "float Damage = CalculateAttackDamage(AttackTarget, bIsCritical)",
    "if (Damage <= 0.0f)",
    "FVector HitLocation = AttackTarget->GetActorLocation()",
    "UDBADamageCalculator::ApplyDamageToTargetWithCue(",
    "Attacker",
    "AttackTarget",
    "Damage",
    "EDBAElement::None",
    "bIsCritical",
    "FGameplayTag::RequestGameplayTag(FName(TEXT(""GameplayCue.DBA.Skill.Impact"")), false)",
    "HitLocation",
    "ClientHitConfirmedWithCritical_Implementation"
  )) {
  Assert-True ($requestAttackImpl.Contains($required)) "Expected ServerRequestAttack_Implementation to contain: $required"
}

$guardIndex = $requestAttackImpl.IndexOf("ValidateServerCharacterContext(")
$targetIndex = $requestAttackImpl.IndexOf("AActor* AttackTarget = FindAttackTarget()")
$missingTargetIndex = $requestAttackImpl.IndexOf("if (!AttackTarget)")
$missingTargetRejectIndex = $requestAttackImpl.IndexOf("ClientHitRejected_Implementation(FGameplayAbilitySpecHandle())")
$damageIndex = $requestAttackImpl.IndexOf("float Damage = CalculateAttackDamage(AttackTarget, bIsCritical)")
$zeroDamageIndex = $requestAttackImpl.IndexOf("if (Damage <= 0.0f)")
$zeroDamageRejectIndex = $requestAttackImpl.LastIndexOf("ClientHitRejected_Implementation(FGameplayAbilitySpecHandle())")
$applyIndex = $requestAttackImpl.IndexOf("UDBADamageCalculator::ApplyDamageToTargetWithCue(")
$confirmIndex = $requestAttackImpl.IndexOf("ClientHitConfirmedWithCritical_Implementation")

Assert-True ($targetIndex -gt $guardIndex) `
  "Expected ServerRequestAttack_Implementation to find a target only after character context validation."
Assert-True ($damageIndex -gt $targetIndex) `
  "Expected ServerRequestAttack_Implementation to calculate damage after finding a target."
Assert-True ($missingTargetRejectIndex -gt $missingTargetIndex -and $missingTargetRejectIndex -lt $damageIndex) `
  "Expected ServerRequestAttack_Implementation to reject the hit when no attack target exists."
Assert-True ($zeroDamageIndex -gt $damageIndex -and $zeroDamageRejectIndex -gt $zeroDamageIndex -and $zeroDamageRejectIndex -lt $applyIndex) `
  "Expected ServerRequestAttack_Implementation to reject non-positive damage before applying damage."
Assert-True ($applyIndex -gt $damageIndex) `
  "Expected ServerRequestAttack_Implementation to apply authoritative damage after calculating damage."
Assert-True ($confirmIndex -gt $applyIndex) `
  "Expected ServerRequestAttack_Implementation to confirm the hit only after authoritative damage is applied."

foreach ($required in @(
    "ValidateServerCharacterContext(",
    "UWorld* World = GetWorld()",
    "if (!World)",
    "return false;",
    "return true;"
  )) {
  Assert-True ($requestAttackValidate.Contains($required)) "Expected ServerRequestAttack_Validate to contain: $required"
}

$validateContextIndex = $requestAttackValidate.IndexOf("ValidateServerCharacterContext(")
$validateWorldIndex = $requestAttackValidate.IndexOf("UWorld* World = GetWorld()")
$validateWorldGuardIndex = $requestAttackValidate.IndexOf("if (!World)")
$validateReturnTrueIndex = $requestAttackValidate.LastIndexOf("return true;")
Assert-True ($validateContextIndex -ge 0 -and $validateWorldIndex -gt $validateContextIndex -and $validateWorldGuardIndex -gt $validateWorldIndex -and $validateReturnTrueIndex -gt $validateWorldGuardIndex) `
  "Expected ServerRequestAttack_Validate to fail closed when World is missing before accepting the RPC."

Write-Host "PASS: RPC handler server attack execution contract" -ForegroundColor Green
