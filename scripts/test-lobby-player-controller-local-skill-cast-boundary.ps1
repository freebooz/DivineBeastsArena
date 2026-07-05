<#
Validates DBALobbyPlayerController only triggers input-driven skill casts from local controllers.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Player\DBALobbyPlayerController.cpp"
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

function Assert-LocalGuardBeforeOperation {
  param(
    [Parameter(Mandatory = $true)][string]$Body,
    [Parameter(Mandatory = $true)][string]$FunctionName,
    [Parameter(Mandatory = $true)][string]$Operation
  )

  $guardIndex = $Body.IndexOf("!IsLocalController()")
  $returnIndex = $Body.IndexOf("return", [Math]::Max(0, $guardIndex))
  $operationIndex = $Body.IndexOf($Operation)

  Assert-True ($guardIndex -ge 0) "Expected $FunctionName to reject non-local controllers before $Operation."
  Assert-True ($returnIndex -gt $guardIndex) "Expected $FunctionName non-local guard to return before $Operation."
  Assert-True ($operationIndex -gt $returnIndex) "Expected $FunctionName to run $Operation only after local-controller guard."
}

$castBody = Get-FunctionBody `
  "void\s+ADBALobbyPlayerController::CastEquippedSkillSlot\s*\(\s*int32\s+SkillSlot\s*\)\s*\{" `
  "`nvoid\s+ADBALobbyPlayerController::HandleSelectTargetPressed" `
  "CastEquippedSkillSlot"

Assert-LocalGuardBeforeOperation $castBody "CastEquippedSkillSlot" "GetPawn()"
Assert-LocalGuardBeforeOperation $castBody "CastEquippedSkillSlot" "ResolveAutoAttackTarget();"
Assert-LocalGuardBeforeOperation $castBody "CastEquippedSkillSlot" "ZodiacPawn->CastEquippedSkillAtTarget"
Assert-LocalGuardBeforeOperation $castBody "CastEquippedSkillSlot" "ZodiacPawn->CastEquippedSkill(SkillSlot);"

Assert-True ($cpp.Contains("HandleSkill01Pressed") -and $cpp.Contains("CastEquippedSkillSlot(1);")) `
  "Expected Skill01 input handler to route through CastEquippedSkillSlot."
Assert-True ($cpp.Contains("HandleUltimatePressed") -and $cpp.Contains("CastEquippedSkillSlot(5);")) `
  "Expected Ultimate input handler to route through CastEquippedSkillSlot."

Write-Host "PASS: Lobby PlayerController local skill cast boundary" -ForegroundColor Green
