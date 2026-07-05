<#
Validates that projectile hit resolution is a C++ runtime entrypoint only.
Blueprints may configure projectile data and presentation assets, but they must
not be able to directly invoke hit resolution or authority-gated damage logic.
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

$onHitDeclarationPattern = "virtual\s+void\s+OnProjectileHit\s*\(\s*AActor\*\s+HitActor,\s*FVector\s+HitLocation\s*\);"
if ($header -notmatch $onHitDeclarationPattern) {
  throw "$headerRelativePath must keep a C++ virtual OnProjectileHit(AActor* HitActor, FVector HitLocation) entrypoint."
}

$headerLines = [System.IO.File]::ReadAllLines($headerPath)
$onHitLineIndex = -1
for ($i = 0; $i -lt $headerLines.Length; $i++) {
  if ($headerLines[$i] -match "virtual\s+void\s+OnProjectileHit\s*\(") {
    $onHitLineIndex = $i
    break
  }
}
if ($onHitLineIndex -lt 0) {
  throw "$headerRelativePath must declare OnProjectileHit."
}

$previousNonEmptyLine = ""
for ($i = $onHitLineIndex - 1; $i -ge 0; $i--) {
  if (-not [string]::IsNullOrWhiteSpace($headerLines[$i])) {
    $previousNonEmptyLine = $headerLines[$i].Trim()
    break
  }
}

if ($previousNonEmptyLine -match "UFUNCTION\s*\([^)]*BlueprintCallable") {
  throw "OnProjectileHit must not be BlueprintCallable; projectile hit resolution is C++ runtime logic."
}

if ($source -notmatch "void\s+ADBASkillProjectileBase::OnProjectileHit\s*\(\s*AActor\*\s+HitActor,\s*FVector\s+HitLocation\s*\)") {
  throw "$sourceRelativePath must define ADBASkillProjectileBase::OnProjectileHit."
}

if ($source -notmatch "HandleProjectile(Hit|Overlap)[\s\S]*?if\s*\(\s*!HasAuthority\(\)\s*\)[\s\S]*?OnProjectileHit\(") {
  throw "$sourceRelativePath must route collision callbacks through OnProjectileHit only after authority checks."
}

Write-Host "PASS: Skill projectile hit entrypoint C++ only" -ForegroundColor Green
