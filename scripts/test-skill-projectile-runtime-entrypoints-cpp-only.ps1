<#
Validates that projectile runtime entrypoints stay in C++.
Blueprints may configure projectile data and presentation assets, but they must
not directly initialize or launch projectile runtime flow.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$headerRelativePath = "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Combat\DBASkillProjectileBase.h"
$headerPath = Join-Path -Path $repoRoot -ChildPath $headerRelativePath

if (-not (Test-Path -LiteralPath $headerPath)) {
  throw "Required projectile header is missing: $headerPath"
}

$header = Get-Content -LiteralPath $headerPath -Encoding UTF8 -Raw
$headerLines = [System.IO.File]::ReadAllLines($headerPath)

function Assert-True {
  param(
    [Parameter(Mandatory = $true)][bool]$Condition,
    [Parameter(Mandatory = $true)][string]$Message
  )

  if (-not $Condition) {
    throw $Message
  }
}

function Get-PreviousNonEmptyLine {
  param(
    [Parameter(Mandatory = $true)][int]$LineIndex
  )

  for ($i = $LineIndex - 1; $i -ge 0; $i--) {
    if (-not [string]::IsNullOrWhiteSpace($headerLines[$i])) {
      return $headerLines[$i].Trim()
    }
  }

  return ""
}

function Assert-EntrypointIsCppOnly {
  param(
    [Parameter(Mandatory = $true)][string]$MethodName,
    [Parameter(Mandatory = $true)][string]$DeclarationPattern
  )

  Assert-True ($header -match $DeclarationPattern) `
    "$headerRelativePath must keep the C++ $MethodName declaration."

  $methodLineIndex = -1
  for ($i = 0; $i -lt $headerLines.Length; $i++) {
    if ($headerLines[$i] -match "\b$MethodName\s*\(") {
      $methodLineIndex = $i
      break
    }
  }

  Assert-True ($methodLineIndex -ge 0) "$headerRelativePath must declare $MethodName."

  $previousNonEmptyLine = Get-PreviousNonEmptyLine $methodLineIndex
  Assert-True (-not ($previousNonEmptyLine -match "UFUNCTION\s*\([^)]*BlueprintCallable")) `
    "$MethodName must not be BlueprintCallable; projectile runtime flow is C++ logic."
}

Assert-EntrypointIsCppOnly `
  "InitializeProjectile" `
  "virtual\s+void\s+InitializeProjectile\s*\("

Assert-EntrypointIsCppOnly `
  "LaunchProjectile" `
  "void\s+LaunchProjectile\s*\(\s*const\s+FVector&\s+Direction\s*\);"

Assert-True ($header -match "UFUNCTION\s*\([^)]*BlueprintCallable[^)]*\)\s*[\r\n\t ]*void\s+SetProjectileProperties\s*\(") `
  "Expected SetProjectileProperties to remain BlueprintCallable as a parameter configuration helper."

Write-Host "PASS: Skill projectile runtime entrypoints C++ only" -ForegroundColor Green
