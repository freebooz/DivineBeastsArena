<#
Validates DBAGameUIManager does not start login flow state machines from unsafe/server-like runtimes.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\DBAGameUIManager.cpp"
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

function Assert-RuntimeGuardBeforeOperation {
  param(
    [Parameter(Mandatory = $true)][string]$Body,
    [Parameter(Mandatory = $true)][string]$FunctionName,
    [Parameter(Mandatory = $true)][string]$Operation
  )

  $worldCacheIndex = $Body.IndexOf("UWorld* World = GetWorld();")
  $safeGuardIndex = $Body.IndexOf("IsWorldSafeForWidgetCreation(World)")
  $serverGuardIndex = $Body.IndexOf("IsServerLikeRuntime(World)")
  $returnIndex = $Body.IndexOf("return", [Math]::Max(0, $serverGuardIndex))
  $operationIndex = $Body.IndexOf($Operation)

  Assert-True ($worldCacheIndex -ge 0) "Expected $FunctionName to cache World before runtime checks."
  Assert-True ($safeGuardIndex -gt $worldCacheIndex) "Expected $FunctionName to check world safety before $Operation."
  Assert-True ($serverGuardIndex -gt $worldCacheIndex) "Expected $FunctionName to check server-like runtime before $Operation."
  Assert-True ($returnIndex -gt $serverGuardIndex) "Expected $FunctionName runtime guard to return before $Operation."
  Assert-True ($operationIndex -gt $returnIndex) "Expected $FunctionName to run $Operation only after runtime guards."
}

$startBody = Get-FunctionBody `
  "void\s+UDBAGameUIManager::EnsureLoginFlowStartedFromManager\s*\(\s*\)\s*\{" `
  "`nvoid\s+UDBAGameUIManager::OnSubsystemDeinitialize" `
  "EnsureLoginFlowStartedFromManager"

Assert-RuntimeGuardBeforeOperation $startBody "EnsureLoginFlowStartedFromManager" "DBAInstance->StartLoginFlow();"
Assert-RuntimeGuardBeforeOperation $startBody "EnsureLoginFlowStartedFromManager" "LoginFlow->StartLoginFlow();"
Assert-RuntimeGuardBeforeOperation $startBody "EnsureLoginFlowStartedFromManager" "bLoginFlowStartRequested = true;"

Write-Host "PASS: Game UI manager login flow start server boundary" -ForegroundColor Green
