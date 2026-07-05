<#
Validates repeated death requests are idempotent.
Once a Zodiac character is Dying or Dead, OnDeath must not replay death
animation, re-enter Dying, or reschedule death finalization.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"
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

$deathBody = Get-FunctionBody `
  "void\s+ADBAZodiacCharacterBase::OnDeath\s*\(" `
  "`nvoid\s+ADBAZodiacCharacterBase::OnRevive" `
  "OnDeath"

foreach ($required in @(
    "if (IsDead())",
    "return;",
    "DeathState = EDADeathState::Dying",
    "PlayDeathAnimation",
    "DeathStateFinalizeTimerHandle ="
  )) {
  Assert-True ($deathBody.Contains($required)) "Expected OnDeath to contain: $required"
}

$authorityIndex = $deathBody.IndexOf("if (!HasAuthority())")
$alreadyDeadIndex = $deathBody.IndexOf("if (IsDead())")
$dyingIndex = $deathBody.IndexOf("DeathState = EDADeathState::Dying")
$animationIndex = $deathBody.IndexOf("PlayDeathAnimation")
$timerIndex = $deathBody.IndexOf("DeathStateFinalizeTimerHandle =")

Assert-True ($authorityIndex -ge 0 -and $alreadyDeadIndex -gt $authorityIndex) `
  "Expected repeated-death guard to run after authority guard."
Assert-True ($dyingIndex -gt $alreadyDeadIndex) `
  "Expected repeated-death guard before writing Dying."
Assert-True ($animationIndex -gt $alreadyDeadIndex) `
  "Expected repeated-death guard before replaying death animation."
Assert-True ($timerIndex -gt $alreadyDeadIndex) `
  "Expected repeated-death guard before rescheduling death finalization."

Write-Host "PASS: Zodiac character death idempotent boundary contract" -ForegroundColor Green
