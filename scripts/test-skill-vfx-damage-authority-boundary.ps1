<#
Validates generic skill VFX damage helpers only mutate gameplay state on the
server authority path. Presentation can be Blueprint-driven, but damage must
remain a C++ authoritative gameplay decision.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$componentPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\VFX\Components\Skill\DBAZodiacSkillVFXComponent_Generic.cpp"
$component = Get-Content -LiteralPath $componentPath -Encoding UTF8 -Raw

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

  $match = [regex]::Match($component, $StartPattern)
  Assert-True $match.Success "Expected $FunctionName implementation."

  $remaining = $component.Substring($match.Index + $match.Length)
  $nextMatch = [regex]::Match($remaining, $EndPattern)
  Assert-True $nextMatch.Success "Expected end boundary after $FunctionName implementation."

  return $component.Substring($match.Index, $match.Length + $nextMatch.Index)
}

$applySkillDamage = Get-FunctionBody `
  "bool\s+UDBAZodiacSkillVFXComponent_Generic::ApplySkillDamage\s*\(" `
  "`nint32\s+UDBAZodiacSkillVFXComponent_Generic::ApplyAOEDamage" `
  "ApplySkillDamage"

$applyAoeDamage = Get-FunctionBody `
  "int32\s+UDBAZodiacSkillVFXComponent_Generic::ApplyAOEDamage\s*\(" `
  "`nvoid\s+UDBAZodiacSkillVFXComponent_Generic::ExecuteSkillGameplayCue" `
  "ApplyAOEDamage"

$playImpactVfx = Get-FunctionBody `
  "void\s+UDBAZodiacSkillVFXComponent_Generic::PlayImpactVFX\s*\(" `
  "`nvoid\s+UDBAZodiacSkillVFXComponent_Generic::PlayProjectileVFX" `
  "PlayImpactVFX"

$playAoeVfx = Get-FunctionBody `
  "void\s+UDBAZodiacSkillVFXComponent_Generic::PlayAOEVFX\s*\(" `
  "`nvoid\s+UDBAZodiacSkillVFXComponent_Generic::PlayChannelVFX" `
  "PlayAOEVFX"

foreach ($required in @(
    "AActor* OwnerActor = GetOwner()",
    "!OwnerActor->HasAuthority()",
    "return false;",
    "CalculateSkillDamage(HitTarget, bOutIsCritical)",
    "UDBADamageCalculator::ApplyDamageToTargetWithCue("
  )) {
  Assert-True ($applySkillDamage.Contains($required)) "Expected ApplySkillDamage to contain: $required"
}

$skillOwnerIndex = $applySkillDamage.IndexOf("AActor* OwnerActor = GetOwner()")
$skillAuthorityIndex = $applySkillDamage.IndexOf("!OwnerActor->HasAuthority()")
$skillCalculateIndex = $applySkillDamage.IndexOf("CalculateSkillDamage(HitTarget, bOutIsCritical)")
$skillApplyIndex = $applySkillDamage.IndexOf("UDBADamageCalculator::ApplyDamageToTargetWithCue(")
Assert-True ($skillOwnerIndex -ge 0 -and $skillAuthorityIndex -gt $skillOwnerIndex -and $skillAuthorityIndex -lt $skillCalculateIndex) `
  "Expected ApplySkillDamage to reject non-authority callers before calculating damage."
Assert-True ($skillApplyIndex -gt $skillCalculateIndex) `
  "Expected ApplySkillDamage to apply damage only after authoritative damage calculation."

foreach ($required in @(
    "UWorld* World = GetWorld()",
    "AActor* OwnerActor = GetOwner()",
    "!OwnerActor->HasAuthority()",
    "return 0;",
    "World->OverlapMultiByObjectType",
    "ApplySkillDamage(HitActor, HitActor->GetActorLocation(), FinalDamage, bIsCritical)"
  )) {
  Assert-True ($applyAoeDamage.Contains($required)) "Expected ApplyAOEDamage to contain: $required"
}

$aoeOwnerIndex = $applyAoeDamage.IndexOf("AActor* OwnerActor = GetOwner()")
$aoeAuthorityIndex = $applyAoeDamage.IndexOf("!OwnerActor->HasAuthority()")
$aoeOverlapIndex = $applyAoeDamage.IndexOf("World->OverlapMultiByObjectType")
$aoeApplyIndex = $applyAoeDamage.IndexOf("ApplySkillDamage(HitActor, HitActor->GetActorLocation(), FinalDamage, bIsCritical)")
Assert-True ($aoeOwnerIndex -ge 0 -and $aoeAuthorityIndex -gt $aoeOwnerIndex -and $aoeAuthorityIndex -lt $aoeOverlapIndex) `
  "Expected ApplyAOEDamage to reject non-authority callers before overlap damage search."
Assert-True ($aoeApplyIndex -gt $aoeOverlapIndex) `
  "Expected ApplyAOEDamage to reuse authoritative ApplySkillDamage for each hit actor."

Assert-True ($playImpactVfx.Contains("ApplySkillDamage(HitTarget, ImpactLocation, FinalDamage, bIsCritical)")) `
  "Expected PlayImpactVFX to route damage through ApplySkillDamage."
Assert-True ($playAoeVfx.Contains("ApplyAOEDamage(Center, EffectiveRadius, HitActors)")) `
  "Expected PlayAOEVFX to route damage through ApplyAOEDamage."

Write-Host "PASS: Skill VFX damage authority boundary contract" -ForegroundColor Green
