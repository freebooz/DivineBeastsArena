<#
Validates ServerLockTarget stores an authoritative locked target and attack
selection prefers that target when it remains valid.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\RPC\DBARpcHandler.h"
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\RPC\DBARpcHandler.cpp"

$header = Get-Content -LiteralPath $headerPath -Encoding UTF8 -Raw
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

$lockTargetImpl = Get-FunctionBody `
  "void\s+ADBARpcHandler::ServerLockTarget_Implementation\s*\(\s*AActor\*\s+TargetActor\s*\)\s*\{" `
  "`nbool\s+ADBARpcHandler::ServerLockTarget_Validate" `
  "ServerLockTarget_Implementation"

$lockTargetValidate = Get-FunctionBody `
  "bool\s+ADBARpcHandler::ServerLockTarget_Validate\s*\(\s*AActor\*\s+TargetActor\s*\)\s*\{" `
  "`nvoid\s+ADBARpcHandler::ServerMoveTo_Implementation" `
  "ServerLockTarget_Validate"

$findAttackTarget = Get-FunctionBody `
  "AActor\*\s+ADBARpcHandler::FindAttackTarget\s*\(\s*\)\s*\{" `
  "`nfloat\s+ADBARpcHandler::CalculateAttackDamage" `
  "FindAttackTarget"

Assert-True ($header.Contains("AActor* GetLockedTargetActor() const")) `
  "Expected DBARpcHandler to expose the current locked target for C++/Blueprint consumers."
Assert-True ($header.Contains("TObjectPtr<AActor> LockedTargetActor")) `
  "Expected DBARpcHandler to store a server-side locked target actor."

foreach ($required in @(
    "ValidateServerCharacterContext(",
    "ValidateTarget(TargetActor)",
    "IsEnemy(GetOwner(), TargetActor)",
    "LockedTargetActor = TargetActor"
  )) {
  Assert-True ($lockTargetImpl.Contains($required)) "Expected ServerLockTarget_Implementation to contain: $required"
}

$contextIndex = $lockTargetImpl.IndexOf("ValidateServerCharacterContext(")
$targetIndex = $lockTargetImpl.IndexOf("ValidateTarget(TargetActor)")
$enemyIndex = $lockTargetImpl.IndexOf("IsEnemy(GetOwner(), TargetActor)")
$assignIndex = $lockTargetImpl.IndexOf("LockedTargetActor = TargetActor")
Assert-True ($contextIndex -ge 0 -and $targetIndex -gt $contextIndex -and $enemyIndex -gt $targetIndex -and $assignIndex -gt $enemyIndex) `
  "Expected ServerLockTarget_Implementation to store target only after context, target, and enemy validation."

foreach ($required in @(
    "ValidateServerCharacterContext(",
    "ValidateTarget(TargetActor)",
    "IsEnemy(GetOwner(), TargetActor)",
    "return false;",
    "return true;"
  )) {
  Assert-True ($lockTargetValidate.Contains($required)) "Expected ServerLockTarget_Validate to contain: $required"
}

$validateContextIndex = $lockTargetValidate.IndexOf("ValidateServerCharacterContext(")
$validateTargetIndex = $lockTargetValidate.IndexOf("ValidateTarget(TargetActor)")
$validateEnemyIndex = $lockTargetValidate.IndexOf("IsEnemy(GetOwner(), TargetActor)")
$validateReturnTrueIndex = $lockTargetValidate.LastIndexOf("return true;")
Assert-True ($validateContextIndex -ge 0 -and $validateTargetIndex -gt $validateContextIndex -and $validateEnemyIndex -gt $validateTargetIndex -and $validateReturnTrueIndex -gt $validateEnemyIndex) `
  "Expected ServerLockTarget_Validate to reject non-enemy targets before accepting the RPC."

foreach ($required in @(
    "AActor* LockedTarget = LockedTargetActor.Get()",
    "ValidateTarget(LockedTarget)",
    "IsEnemy(OwnerActor, LockedTarget)",
    "ValidateCastRange(LockedTarget, DBAConstants::DefaultAttackRange)",
    "return LockedTarget"
  )) {
  Assert-True ($findAttackTarget.Contains($required)) "Expected FindAttackTarget to prefer locked target with: $required"
}

$lockedTargetIndex = $findAttackTarget.IndexOf("AActor* LockedTarget = LockedTargetActor.Get()")
$overlapIndex = $findAttackTarget.IndexOf("OverlapMultiByObjectType")
Assert-True ($lockedTargetIndex -ge 0 -and $overlapIndex -gt $lockedTargetIndex) `
  "Expected FindAttackTarget to evaluate locked target before overlap fallback."

Write-Host "PASS: RPC handler server lock target execution contract" -ForegroundColor Green
