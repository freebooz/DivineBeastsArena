<#
Validates chain lightning damage cannot bypass the server-authoritative path.
The spell actor may expose BlueprintCallable casting for configuration/presentation,
but damage application must remain guarded in C++.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBAChainLightningSpell.cpp"
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBAChainLightningSpell.h"
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

Assert-True ($header.Contains("UFUNCTION(BlueprintCallable, Category = ""DBA|Chain Lightning"")")) `
  "Expected Chain Lightning cast entrypoint to remain BlueprintCallable for configuration/presentation shell usage."

$castBody = Get-FunctionBody `
  "void\s+ADBAChainLightningSpell::CastChainLightning\s*\(" `
  "`nAActor\*\s+ADBAChainLightningSpell::FindInitialTarget" `
  "CastChainLightning"

foreach ($required in @(
    "if (!World || !InCaster)",
    "if (!HasAuthority() && GetNetMode() != NM_Standalone)",
    "return;",
    "ApplyChainDamage(InCaster, CurrentTarget, CurrentTargetLocation, JumpIndex)",
    "MulticastPlayChainLightning(Sources, Targets, SegmentScales)"
  )) {
  Assert-True ($castBody.Contains($required)) "Expected CastChainLightning to contain: $required"
}

$castAuthorityIndex = $castBody.IndexOf("if (!HasAuthority() && GetNetMode() != NM_Standalone)")
$castDamageIndex = $castBody.IndexOf("ApplyChainDamage(InCaster, CurrentTarget, CurrentTargetLocation, JumpIndex)")
Assert-True ($castAuthorityIndex -ge 0 -and $castDamageIndex -gt $castAuthorityIndex) `
  "Expected CastChainLightning to reject non-authority network callers before chain damage is applied."

$damageBody = Get-FunctionBody `
  "void\s+ADBAChainLightningSpell::ApplyChainDamage\s*\(" `
  "`nvoid\s+ADBAChainLightningSpell::StartLocalSequence" `
  "ApplyChainDamage"

foreach ($required in @(
    "if (!HasAuthority())",
    "return;",
    "if (!Caster || !Target || BaseDamage <= 0.0f)",
    "const float DamageScale = FMath::Pow",
    "UDBADamageCalculator::ApplyDamageToTargetWithCue("
  )) {
  Assert-True ($damageBody.Contains($required)) "Expected ApplyChainDamage to contain: $required"
}

$damageAuthorityIndex = $damageBody.IndexOf("if (!HasAuthority())")
$damageScaleIndex = $damageBody.IndexOf("const float DamageScale = FMath::Pow")
$applyDamageIndex = $damageBody.IndexOf("UDBADamageCalculator::ApplyDamageToTargetWithCue(")
Assert-True ($damageAuthorityIndex -ge 0 -and $damageScaleIndex -gt $damageAuthorityIndex -and $applyDamageIndex -gt $damageScaleIndex) `
  "Expected ApplyChainDamage to guard authority before calculating or applying chain damage."

Write-Host "PASS: Chain Lightning damage authority boundary contract" -ForegroundColor Green
