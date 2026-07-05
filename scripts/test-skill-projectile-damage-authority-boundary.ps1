<#
Validates projectile hit damage remains server-authoritative. Projectile
configuration can be BlueprintCallable, but hit resolution stays C++ only.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBASkillProjectileBase.cpp"
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBASkillProjectileBase.h"
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

Assert-True ($header.Contains("void SetProjectileProperties(float InSpeed, float InRadius, float InDamage);")) `
  "Expected projectile configuration helpers to remain available."
Assert-True (-not ($header -match "UFUNCTION\s*\([^)]*BlueprintCallable[^)]*\)\s*[\r\n\t ]*virtual\s+void\s+OnProjectileHit")) `
  "Expected OnProjectileHit to stay C++ only, not BlueprintCallable."

$onProjectileHit = Get-FunctionBody `
  "void\s+ADBASkillProjectileBase::OnProjectileHit\s*\(" `
  "`nvoid\s+ADBASkillProjectileBase::MulticastPlayImpactFeedback_Implementation" `
  "OnProjectileHit"

foreach ($required in @(
    "PlayImpactFeedbackLocal(",
    "if (HasAuthority() && GetNetMode() != NM_Standalone)",
    "MulticastPlayImpactFeedback(",
    "if (HasAuthority() && HitActor && HitActor != ProjectileOwner && Damage > 0.0f)",
    "UDBADamageCalculator::CalculateFinalDamage(",
    "UDBADamageCalculator::ApplyDamageToTargetWithCue(",
    "OnProjectileHitResolved(HitActor, HitLocation)",
    "Destroy()"
  )) {
  Assert-True ($onProjectileHit.Contains($required)) "Expected OnProjectileHit to contain: $required"
}

$feedbackIndex = $onProjectileHit.IndexOf("PlayImpactFeedbackLocal(")
$damageAuthorityIndex = $onProjectileHit.IndexOf("if (HasAuthority() && HitActor && HitActor != ProjectileOwner && Damage > 0.0f)")
$damageCalcIndex = $onProjectileHit.IndexOf("UDBADamageCalculator::CalculateFinalDamage(")
$applyDamageIndex = $onProjectileHit.IndexOf("UDBADamageCalculator::ApplyDamageToTargetWithCue(")
$resolvedHookIndex = $onProjectileHit.IndexOf("OnProjectileHitResolved(HitActor, HitLocation)")
$destroyIndex = $onProjectileHit.IndexOf("Destroy()")

Assert-True ($feedbackIndex -ge 0 -and $damageAuthorityIndex -gt $feedbackIndex) `
  "Expected impact feedback to remain available before authority-gated damage handling."
Assert-True ($damageCalcIndex -gt $damageAuthorityIndex -and $applyDamageIndex -gt $damageCalcIndex) `
  "Expected projectile damage calculation and application to be inside the authority-gated damage block."
Assert-True ($resolvedHookIndex -gt $applyDamageIndex -and $destroyIndex -gt $resolvedHookIndex) `
  "Expected C++ post-hit hook and Destroy to run after the authority-gated damage block."

$hitHandler = Get-FunctionBody `
  "void\s+ADBASkillProjectileBase::HandleProjectileHit\s*\(" `
  "`nvoid\s+ADBASkillProjectileBase::HandleProjectileOverlap" `
  "HandleProjectileHit"
Assert-True ($hitHandler.Contains("if (!HasAuthority())")) "Expected HandleProjectileHit to ignore non-authority collision callbacks."
Assert-True ($hitHandler.Contains("OnProjectileHit(")) "Expected HandleProjectileHit to route authority hits through OnProjectileHit."

$overlapHandler = Get-FunctionBody `
  "void\s+ADBASkillProjectileBase::HandleProjectileOverlap\s*\(" `
  "$" `
  "HandleProjectileOverlap"
Assert-True ($overlapHandler.Contains("if (!HasAuthority())")) "Expected HandleProjectileOverlap to ignore non-authority collision callbacks."
Assert-True ($overlapHandler.Contains("OnProjectileHit(")) "Expected HandleProjectileOverlap to route authority hits through OnProjectileHit."

Write-Host "PASS: Skill projectile damage authority boundary contract" -ForegroundColor Green
