<#
Validates that generic GAS ability lifecycle logic stays in C++.
Blueprints may configure SkillID/DataTable assets, but they must not implement
ability activation/end hooks for generic gameplay abilities.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$abilityRoot = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\Abilities"

if (-not (Test-Path -LiteralPath $abilityRoot)) {
  throw "GAS ability header root is missing: $abilityRoot"
}

$targets = @(
  "DBAElementSkillAbility_Generic.h",
  "DBAZodiacUltimateAbility_Generic.h",
  "DBAZodiacPassiveAbility_Generic.h"
)

$violations = New-Object System.Collections.Generic.List[string]
foreach ($target in $targets) {
  $path = Join-Path -Path $abilityRoot -ChildPath $target
  if (-not (Test-Path -LiteralPath $path)) {
    throw "Required GAS ability header is missing: $target"
  }

  $matches = Select-String -LiteralPath $path -Pattern "Blueprint(Implementable|Native)Event"
  foreach ($match in $matches) {
    $violations.Add(("{0}:{1}: {2}" -f $target, $match.LineNumber, $match.Line.Trim()))
  }
}

if ($violations.Count -gt 0) {
  throw "Generic GAS ability lifecycle hooks must be implemented in C++, not Blueprint:`n$($violations -join "`n")"
}

foreach ($target in $targets) {
  $path = Join-Path -Path $abilityRoot -ChildPath $target
  $content = Get-Content -LiteralPath $path -Encoding UTF8 -Raw
  if ($content -notmatch "virtual\s+void\s+On[A-Za-z0-9_]+\(") {
    throw "$target must expose C++ virtual lifecycle hook methods."
  }
}

Write-Host "PASS: GAS ability C++ lifecycle boundary" -ForegroundColor Green
