<#
Validates monster AI movement and patrol runtime state are server-authoritative.
BlueprintCallable movement helpers may remain C++ bridge entrypoints, but clients
must not drive AIController movement, mutate patrol cursors, or rewrite spawn state.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\Monster\AI\DBAMonsterAIComponent.cpp"
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\Monster\AI\DBAMonsterAIComponent.h"
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

$expectedBridgeEntryPoints = @(
  @{ Name = "MoveToLocation"; Category = "AI\|Movement"; ReturnType = "void" },
  @{ Name = "MoveToActor"; Category = "AI\|Movement"; ReturnType = "void" },
  @{ Name = "StopMovement"; Category = "AI\|Movement"; ReturnType = "void" },
  @{ Name = "GetNextPatrolPoint"; Category = "AI\|Patrol"; ReturnType = "FVector" },
  @{ Name = "SetSpawnLocation"; Category = "AI"; ReturnType = "void" }
)

foreach ($entrypoint in $expectedBridgeEntryPoints) {
  Assert-True ($header -match "UFUNCTION\(BlueprintCallable,\s*Category\s*=\s*""$($entrypoint.Category)""\)[\s\S]{0,160}$($entrypoint.ReturnType)\s+$($entrypoint.Name)\s*\(") `
    "Expected $($entrypoint.Name) to remain a BlueprintCallable C++ AI bridge entrypoint."
}

Assert-True ($header -notmatch "void\s+SetSpawnLocation\s*\(\s*FVector\s+Location\s*\)\s*\{\s*SpawnLocation\s*=\s*Location;\s*\}") `
  "Expected SetSpawnLocation to use a guarded C++ implementation instead of inline state mutation."

$functions = @(
  @{
    Name = "MoveToLocation"
    Start = "void\s+UDBAMonsterAIComponent::MoveToLocation\s*\("
    End = "`nvoid\s+UDBAMonsterAIComponent::MoveToActor"
    Writes = @("AIController->MoveTo", "Movement->RequestDirectMove")
  },
  @{
    Name = "MoveToActor"
    Start = "void\s+UDBAMonsterAIComponent::MoveToActor\s*\("
    End = "`nvoid\s+UDBAMonsterAIComponent::StopMovement"
    Writes = @("AIController->MoveTo")
  },
  @{
    Name = "StopMovement"
    Start = "void\s+UDBAMonsterAIComponent::StopMovement\s*\("
    End = "`nbool\s+UDBAMonsterAIComponent::IsMoving"
    Writes = @("AIController->StopMovement", "Movement->Velocity = FVector::ZeroVector")
  },
  @{
    Name = "GetNextPatrolPoint"
    Start = "FVector\s+UDBAMonsterAIComponent::GetNextPatrolPoint\s*\("
    End = "`nvoid\s+UDBAMonsterAIComponent::UpdateAggroList"
    Writes = @("CurrentPatrolIndex =")
  },
  @{
    Name = "SetSpawnLocation"
    Start = "void\s+UDBAMonsterAIComponent::SetSpawnLocation\s*\("
    End = "`nvoid\s+UDBAMonsterAIComponent::UpdateAggroList"
    Writes = @("SpawnLocation = Location")
  }
)

foreach ($function in $functions) {
  $body = Get-FunctionBody $function.Start $function.End $function.Name
  Assert-True ($body.Contains("HasAuthority()")) `
    "Expected $($function.Name) to explicitly fail closed for non-authority callers."
  Assert-True ($body.Contains("return")) `
    "Expected $($function.Name) authority guard to return before monster AI movement or patrol mutation."

  $guardIndex = $body.IndexOf("HasAuthority()")
  foreach ($write in $function.Writes) {
    $writeIndex = $body.IndexOf($write)
    Assert-True ($writeIndex -gt $guardIndex) `
      "Expected $($function.Name) to guard authority before: $write"
  }
}

Write-Host "PASS: Monster AI movement authority boundary contract" -ForegroundColor Green
