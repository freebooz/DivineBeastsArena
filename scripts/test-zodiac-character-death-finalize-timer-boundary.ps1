<#
Validates Zodiac character death finalization cannot override a same-frame revive.
Death can still pass through Dying for animation, but the delayed transition to
Dead must be cancelable and must re-check the current state before mutating.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h"
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

Assert-True ($header.Contains("FTimerHandle DeathStateFinalizeTimerHandle")) `
  "Expected a retained timer handle for cancelable death-state finalization."

$deathBody = Get-FunctionBody `
  "void\s+ADBAZodiacCharacterBase::OnDeath\s*\(" `
  "`nvoid\s+ADBAZodiacCharacterBase::OnRevive" `
  "OnDeath"

foreach ($required in @(
    "ClearTimer(DeathStateFinalizeTimerHandle)",
    "DeathStateFinalizeTimerHandle =",
    "SetTimerForNextTick",
    "FTimerDelegate::CreateWeakLambda",
    "DeathState != EDADeathState::Dying",
    "DeathState = EDADeathState::Dead"
  )) {
  Assert-True ($deathBody.Contains($required)) "Expected OnDeath to contain: $required"
}

$dyingIndex = $deathBody.IndexOf("DeathState = EDADeathState::Dying")
$timerIndex = $deathBody.IndexOf("DeathStateFinalizeTimerHandle =")
$deadIndex = $deathBody.IndexOf("DeathState = EDADeathState::Dead")
Assert-True ($dyingIndex -ge 0 -and $timerIndex -gt $dyingIndex -and $deadIndex -gt $timerIndex) `
  "Expected death finalization to be scheduled after entering Dying."

$reviveBody = Get-FunctionBody `
  "void\s+ADBAZodiacCharacterBase::OnRevive\s*\(" `
  "`nvoid\s+ADBAZodiacCharacterBase::UpdateSkillCooldowns" `
  "OnRevive"

foreach ($required in @(
    "ClearTimer(DeathStateFinalizeTimerHandle)",
    "DeathState = EDADeathState::Alive"
  )) {
  Assert-True ($reviveBody.Contains($required)) "Expected OnRevive to contain: $required"
}

$clearIndex = $reviveBody.IndexOf("ClearTimer(DeathStateFinalizeTimerHandle)")
$aliveIndex = $reviveBody.IndexOf("DeathState = EDADeathState::Alive")
Assert-True ($clearIndex -ge 0 -and $aliveIndex -gt $clearIndex) `
  "Expected OnRevive to cancel pending death finalization before setting Alive."

Write-Host "PASS: Zodiac character death finalize timer boundary contract" -ForegroundColor Green
