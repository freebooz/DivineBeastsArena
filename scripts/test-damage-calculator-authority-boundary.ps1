<#
Validates central damage application cannot bypass the server-authoritative path.
Damage helpers may remain BlueprintCallable as C++-owned configuration bridges,
but gameplay state mutation must fail closed for non-authority attackers.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBADamageCalculator.cpp"
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBADamageCalculator.h"
$cpp = Get-Content -LiteralPath $cppPath -Encoding UTF8 -Raw
$header = Get-Content -LiteralPath $headerPath -Encoding UTF8 -Raw

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

Assert-True ($header.Contains("UFUNCTION(BlueprintCallable, Category = ""DBA|Damage"")")) `
  "Expected damage application helpers to remain BlueprintCallable C++ bridge entrypoints."

$applyDamageBody = Get-FunctionBody `
  "void\s+UDBADamageCalculator::ApplyDamageToTarget\s*\(" `
  "`nvoid\s+UDBADamageCalculator::ApplyDamageToTargetWithCue" `
  "ApplyDamageToTarget"

foreach ($required in @(
    "if (!Target || !Attacker)",
    "if (!Attacker->HasAuthority())",
    "return;",
    "UAbilitySystemComponent* TargetASC",
    "BattleAttrSet->SetCurrentHealth",
    "ZodiacChar->OnDeath()"
  )) {
  Assert-True ($applyDamageBody.Contains($required)) "Expected ApplyDamageToTarget to contain: $required"
}

$plainAuthorityIndex = $applyDamageBody.IndexOf("if (!Attacker->HasAuthority())")
$plainAscIndex = $applyDamageBody.IndexOf("UAbilitySystemComponent* TargetASC")
$plainHealthIndex = $applyDamageBody.IndexOf("BattleAttrSet->SetCurrentHealth")
$plainDeathIndex = $applyDamageBody.IndexOf("ZodiacChar->OnDeath()")
Assert-True ($plainAuthorityIndex -ge 0 -and $plainAscIndex -gt $plainAuthorityIndex -and $plainHealthIndex -gt $plainAuthorityIndex -and $plainDeathIndex -gt $plainAuthorityIndex) `
  "Expected ApplyDamageToTarget to guard attacker authority before ASC mutation or death handling."

$applyDamageWithCueBody = Get-FunctionBody `
  "void\s+UDBADamageCalculator::ApplyDamageToTargetWithCue\s*\(" `
  "`nEDBAElementType\s+UDBADamageCalculator::GetElementTypeFromOldEnum" `
  "ApplyDamageToTargetWithCue"

foreach ($required in @(
    "if (!Target || !Attacker || FinalDamage <= 0.0f)",
    "if (!Attacker->HasAuthority())",
    "return;",
    "UAbilitySystemComponent* TargetASC",
    "BattleAttrSet->SetCurrentHealth",
    "Target->TakeDamage",
    "ExecuteDamageGameplayCue",
    "ZodiacChar->OnDeath()"
  )) {
  Assert-True ($applyDamageWithCueBody.Contains($required)) "Expected ApplyDamageToTargetWithCue to contain: $required"
}

$cueAuthorityIndex = $applyDamageWithCueBody.IndexOf("if (!Attacker->HasAuthority())")
$cueAscIndex = $applyDamageWithCueBody.IndexOf("UAbilitySystemComponent* TargetASC")
$cueTakeDamageIndex = $applyDamageWithCueBody.IndexOf("Target->TakeDamage")
$cueGameplayCueIndex = $applyDamageWithCueBody.IndexOf("ExecuteDamageGameplayCue")
$cueDeathIndex = $applyDamageWithCueBody.IndexOf("ZodiacChar->OnDeath()")
Assert-True ($cueAuthorityIndex -ge 0 -and $cueAscIndex -gt $cueAuthorityIndex -and $cueTakeDamageIndex -gt $cueAuthorityIndex -and $cueGameplayCueIndex -gt $cueAuthorityIndex -and $cueDeathIndex -gt $cueAuthorityIndex) `
  "Expected ApplyDamageToTargetWithCue to guard attacker authority before damage, cues, or death handling."

Write-Host "PASS: DamageCalculator authority boundary contract" -ForegroundColor Green
