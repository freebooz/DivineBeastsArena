<#
Validates that skill projectile hit lifecycle extension stays in C++.
Blueprints may configure projectile parameters and presentation assets, but hit
resolution hooks must not be Blueprint-implemented gameplay events.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$headerRelativePath = "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBASkillProjectileBase.h"
$sourceRelativePath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Combat\DBASkillProjectileBase.cpp"
$headerPath = Join-Path -Path $repoRoot -ChildPath $headerRelativePath
$sourcePath = Join-Path -Path $repoRoot -ChildPath $sourceRelativePath

foreach ($path in @($headerPath, $sourcePath)) {
  if (-not (Test-Path -LiteralPath $path)) {
    throw "Required projectile source file is missing: $path"
  }
}

$header = Get-Content -LiteralPath $headerPath -Encoding UTF8 -Raw
$source = Get-Content -LiteralPath $sourcePath -Encoding UTF8 -Raw

$forbidden = @(
  "BlueprintImplementableEvent",
  "BlueprintNativeEvent",
  "BP_OnProjectileHit"
)

$presentForbidden = @($forbidden | Where-Object { $header.Contains($_) -or $source.Contains($_) })
if ($presentForbidden.Count -gt 0) {
  throw "Skill projectile hit lifecycle must use C++ hooks, not Blueprint events: $($presentForbidden -join ', ')"
}

if ($header -notmatch "virtual\s+void\s+OnProjectileHitResolved\s*\(\s*AActor\*\s+HitActor,\s*FVector\s+HitLocation\s*\)") {
  throw "$headerRelativePath must declare virtual void OnProjectileHitResolved(AActor* HitActor, FVector HitLocation)."
}

if ($source -notmatch "void\s+ADBASkillProjectileBase::OnProjectileHitResolved\s*\(\s*AActor\*\s+HitActor,\s*FVector\s+HitLocation\s*\)") {
  throw "$sourceRelativePath must define ADBASkillProjectileBase::OnProjectileHitResolved."
}

$hookIndex = $source.IndexOf("OnProjectileHitResolved(HitActor, HitLocation);")
$destroyIndex = $source.IndexOf("Destroy();")
if ($hookIndex -lt 0) {
  throw "$sourceRelativePath must call OnProjectileHitResolved(HitActor, HitLocation) during hit resolution."
}
if ($destroyIndex -lt 0 -or $hookIndex -gt $destroyIndex) {
  throw "$sourceRelativePath must invoke OnProjectileHitResolved before Destroy()."
}

Write-Host "PASS: Skill projectile C++ hit boundary" -ForegroundColor Green
