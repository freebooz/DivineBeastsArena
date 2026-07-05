<#
Validates FindAttackTarget clears stale locked targets before falling back to
overlap target search.
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

Assert-True ($header.Contains("AActor* FindAttackTarget();")) `
  "Expected FindAttackTarget to be non-const so it can clear stale locked target state."

$findAttackTarget = Get-FunctionBody `
  "AActor\*\s+ADBARpcHandler::FindAttackTarget\s*\(\s*\)\s*\{" `
  "`nfloat\s+ADBARpcHandler::CalculateAttackDamage" `
  "FindAttackTarget"

foreach ($required in @(
    "AActor* LockedTarget = LockedTargetActor.Get()",
    "ValidateTarget(LockedTarget)",
    "IsEnemy(OwnerActor, LockedTarget)",
    "ValidateCastRange(LockedTarget, DBAConstants::DefaultAttackRange)",
    "if (LockedTarget)",
    "LockedTargetActor = nullptr",
    "OverlapMultiByObjectType"
  )) {
  Assert-True ($findAttackTarget.Contains($required)) "Expected FindAttackTarget to contain: $required"
}

$lockedTargetIndex = $findAttackTarget.IndexOf("AActor* LockedTarget = LockedTargetActor.Get()")
$returnLockedIndex = $findAttackTarget.IndexOf("return LockedTarget")
$clearConditionIndex = $findAttackTarget.IndexOf("if (LockedTarget)")
$clearIndex = $findAttackTarget.IndexOf("LockedTargetActor = nullptr")
$overlapIndex = $findAttackTarget.IndexOf("OverlapMultiByObjectType")

Assert-True ($returnLockedIndex -gt $lockedTargetIndex) `
  "Expected valid locked target to be returned before stale cleanup."
Assert-True ($clearConditionIndex -gt $returnLockedIndex) `
  "Expected stale locked target cleanup to run only after the valid locked target fast path fails."
Assert-True ($clearIndex -gt $clearConditionIndex) `
  "Expected stale locked target cleanup to clear LockedTargetActor."
Assert-True ($overlapIndex -gt $clearIndex) `
  "Expected stale locked target cleanup before overlap fallback search."

Write-Host "PASS: RPC handler stale locked target clear contract" -ForegroundColor Green
