<#
Checks whether a production-like cooked/staged Unreal Dedicated Server package is present.

This script is read-only by default. Use -RunSmoke only after a staged package is found.
#>

[CmdletBinding()]
param(
  [string]$ProjectPath = "",
  [string]$PackagedRoot = "",
  [string]$ServerExePath = "",
  [switch]$RunSmoke,
  [int]$SmokeRunSeconds = 20,
  [int]$Port = 17779
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
  $ProjectPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\DivineBeastsArena.uproject"
}

$projectFullPath = (Resolve-Path -LiteralPath $ProjectPath).ProviderPath
$projectDir = Split-Path -Parent $projectFullPath

function Write-Check {
  param(
    [string]$Name,
    [bool]$Passed,
    [string]$Detail
  )

  if ($Passed) {
    Write-Host "PASS $Name - $Detail" -ForegroundColor Green
  }
  else {
    Write-Host "FAIL $Name - $Detail" -ForegroundColor Red
  }
}

function Find-ServerPackages {
  param(
    [string]$ExplicitPackagedRoot,
    [string]$ExplicitServerExePath,
    [string]$ProjectDirectory,
    [string]$RepositoryRoot
  )

  if (-not [string]::IsNullOrWhiteSpace($ExplicitServerExePath)) {
    if (Test-Path -LiteralPath $ExplicitServerExePath) {
      return @((Resolve-Path -LiteralPath $ExplicitServerExePath).ProviderPath)
    }
    return @()
  }

  $roots = @()
  if (-not [string]::IsNullOrWhiteSpace($ExplicitPackagedRoot)) {
    $roots += $ExplicitPackagedRoot
  }
  $roots += @(
    (Join-Path -Path $RepositoryRoot -ChildPath ".tmp\packaged-server"),
    (Join-Path -Path $ProjectDirectory -ChildPath "Saved\StagedBuilds"),
    (Join-Path -Path $RepositoryRoot -ChildPath "Artifacts\UnrealServer")
  )

  $found = @()
  foreach ($root in $roots) {
    if (Test-Path -LiteralPath $root) {
      $found += Get-ChildItem -LiteralPath $root -Recurse -File -Filter "DivineBeastsArenaServer.exe" -ErrorAction SilentlyContinue |
        Select-Object -ExpandProperty FullName
    }
  }

  return @($found | Sort-Object -Unique)
}

function Test-CookedContentNear {
  param([string]$ServerExe)

  $cursor = Split-Path -Parent $ServerExe
  for ($i = 0; $i -lt 8 -and -not [string]::IsNullOrWhiteSpace($cursor); $i++) {
    $pak = Get-ChildItem -LiteralPath $cursor -Recurse -File -Filter "*.pak" -ErrorAction SilentlyContinue | Select-Object -First 1
    $assetRegistry = Get-ChildItem -LiteralPath $cursor -Recurse -File -Filter "AssetRegistry.bin" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($pak -or $assetRegistry) {
      return $true
    }

    $parent = Split-Path -Parent $cursor
    if ($parent -eq $cursor) {
      break
    }
    $cursor = $parent
  }

  return $false
}

$rawServerExe = Join-Path -Path $projectDir -ChildPath "Binaries\Win64\DivineBeastsArenaServer.exe"
Write-Check "compiled-server-target" (Test-Path -LiteralPath $rawServerExe) $rawServerExe

$serverPackages = Find-ServerPackages `
  -ExplicitPackagedRoot $PackagedRoot `
  -ExplicitServerExePath $ServerExePath `
  -ProjectDirectory $projectDir `
  -RepositoryRoot $repoRoot

Write-Check "staged-server-executable" ($serverPackages.Count -gt 0) "found $($serverPackages.Count) candidate(s)"

$readyPackages = @()
foreach ($candidate in $serverPackages) {
  $hasCookedContent = Test-CookedContentNear -ServerExe $candidate
  Write-Check "cooked-content" $hasCookedContent $candidate
  if ($hasCookedContent) {
    $readyPackages += $candidate
  }
}

if ($readyPackages.Count -eq 0) {
  Write-Host "SUMMARY: packaged server is not ready. A compiled raw server target is not enough; cook and stage server content first." -ForegroundColor Yellow
  exit 1
}

Write-Host "SUMMARY: packaged server readiness checks passed." -ForegroundColor Green
Write-Host "ServerExePath: $($readyPackages[0])"

if ($RunSmoke) {
  & (Join-Path -Path $PSScriptRoot -ChildPath "smoke-unreal-dedicated-server.ps1") `
    -UsePackagedServer `
    -ServerExePath $readyPackages[0] `
    -RunSeconds $SmokeRunSeconds `
    -Port $Port
}
