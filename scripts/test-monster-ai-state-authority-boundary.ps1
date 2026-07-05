<#
Validates replicated monster AI state can only be mutated by authority.
BlueprintCallable AI commands are kept as C++ bridge entrypoints, but replicated
CurrentState, CurrentTarget, and AggroList writes must fail closed on clients.
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

foreach ($entrypoint in @("TransitionTo", "FindTarget", "ClearTarget", "AttackTarget", "AddAggro", "RemoveAggro", "ClearAggroList", "UpdateAggroList")) {
  Assert-True ($header -match "UFUNCTION\(BlueprintCallable,\s*Category\s*=\s*""AI(?:\|Aggro)?""\)[\s\S]{0,140}(?:void|AActor\*|bool)\s+$entrypoint\s*\(") `
    "Expected $entrypoint to remain a BlueprintCallable C++ AI bridge entrypoint."
}

$functions = @(
  @{
    Name = "TransitionTo"
    Start = "void\s+UDBAMonsterAIComponent::TransitionTo\s*\("
    End = "`nvoid\s+UDBAMonsterAIComponent::FindTarget"
    Writes = @("CurrentState = NewState")
  },
  @{
    Name = "FindTarget"
    Start = "void\s+UDBAMonsterAIComponent::FindTarget\s*\("
    End = "`nvoid\s+UDBAMonsterAIComponent::ClearTarget"
    Writes = @("CurrentTarget = BestTarget", "TransitionTo(CurrentTarget ?")
  },
  @{
    Name = "ClearTarget"
    Start = "void\s+UDBAMonsterAIComponent::ClearTarget\s*\("
    End = "`nvoid\s+UDBAMonsterAIComponent::AttackTarget"
    Writes = @("CurrentTarget = nullptr", "TransitionTo(EMonsterAIState::Idle)")
  },
  @{
    Name = "AttackTarget"
    Start = "void\s+UDBAMonsterAIComponent::AttackTarget\s*\("
    End = "`nbool\s+UDBAMonsterAIComponent::IsInAttackRange"
    Writes = @("LastAttackTime =")
  },
  @{
    Name = "AddAggro"
    Start = "void\s+UDBAMonsterAIComponent::AddAggro\s*\("
    End = "`nvoid\s+UDBAMonsterAIComponent::RemoveAggro"
    Writes = @("Info.AddThreat", "AggroList.Add")
  },
  @{
    Name = "RemoveAggro"
    Start = "void\s+UDBAMonsterAIComponent::RemoveAggro\s*\("
    End = "`nAActor\*\s+UDBAMonsterAIComponent::GetTopAggroTarget"
    Writes = @("AggroList.RemoveAll")
  },
  @{
    Name = "ClearAggroList"
    Start = "void\s+UDBAMonsterAIComponent::ClearAggroList\s*\("
    End = "`nint32\s+UDBAMonsterAIComponent::GetPatrolPointCount"
    Writes = @("AggroList.Empty")
  },
  @{
    Name = "UpdateAggroList"
    Start = "void\s+UDBAMonsterAIComponent::UpdateAggroList\s*\("
    End = "`nbool\s+UDBAMonsterAIComponent::HasLineOfSightTo"
    Writes = @("AggroList.RemoveAll", "LastAggroUpdateTime =")
  },
  @{
    Name = "RefreshAggroTarget"
    Start = "void\s+UDBAMonsterAIComponent::RefreshAggroTarget\s*\("
    End = "`nvoid\s+UDBAMonsterAIComponent::OnRep_CurrentState"
    Writes = @("CurrentTarget = GetTopAggroTarget")
  }
)

foreach ($function in $functions) {
  $body = Get-FunctionBody $function.Start $function.End $function.Name
  Assert-True ($body.Contains("HasAuthority()")) `
    "Expected $($function.Name) to explicitly fail closed for non-authority callers."
  Assert-True ($body.Contains("return")) `
    "Expected $($function.Name) authority guard to return before monster AI state mutation."

  $guardIndex = $body.IndexOf("HasAuthority()")
  foreach ($write in $function.Writes) {
    $writeIndex = $body.IndexOf($write)
    Assert-True ($writeIndex -gt $guardIndex) `
      "Expected $($function.Name) to guard authority before: $write"
  }
}

Write-Host "PASS: Monster AI state authority boundary contract" -ForegroundColor Green
