<#
Validates Zodiac character server-side equipped skill RPC implementations guard authority, slot, death, and target state.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\DBAZodiacCharacterBase.h"
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"

$header = Get-Content -LiteralPath $headerPath -Encoding UTF8 -Raw
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

function Assert-GuardBeforeCast {
  param(
    [Parameter(Mandatory = $true)][string]$Body,
    [Parameter(Mandatory = $true)][string]$FunctionName
  )

  $guardIndex = $Body.IndexOf("!ValidateServerEquippedSkillCast(")
  $returnIndex = $Body.IndexOf("return", [Math]::Max(0, $guardIndex))
  $castIndex = $Body.IndexOf("CastEquippedSkillInternal")

  Assert-True ($guardIndex -ge 0) "Expected $FunctionName to validate server equipped skill cast before internal cast."
  Assert-True ($returnIndex -gt $guardIndex) "Expected $FunctionName validation guard to return before internal cast."
  Assert-True ($castIndex -gt $returnIndex) "Expected $FunctionName to call CastEquippedSkillInternal only after validation."
}

Assert-True ($header -match "bool\s+ValidateServerEquippedSkillCast\s*\(\s*int32\s+SkillSlot,\s*AActor\*\s+TargetActor\s*\)\s+const\s*;") `
  "Expected Zodiac character to declare reusable server equipped skill cast validation."

$validatorBody = Get-FunctionBody `
  "bool\s+ADBAZodiacCharacterBase::ValidateServerEquippedSkillCast\s*\(\s*int32\s+SkillSlot,\s*AActor\*\s+TargetActor\s*\)\s+const\s*\{" `
  "`nvoid\s+ADBAZodiacCharacterBase::ServerCastLobbyFireball_Implementation" `
  "ValidateServerEquippedSkillCast"

Assert-True ($validatorBody.Contains("HasAuthority()")) "Expected server cast validator to require authority."
Assert-True ($validatorBody.Contains("GetWorld()")) "Expected server cast validator to require a valid World."
Assert-True ($validatorBody.Contains("IsLobbyEquippedSkillSlot(SkillSlot)")) "Expected server cast validator to reject invalid equipped skill slots."
Assert-True ($validatorBody.Contains("IsDead()")) "Expected server cast validator to reject dead or dying characters."
Assert-True ($validatorBody.Contains("TargetActor && !IsValid(TargetActor)")) "Expected server cast validator to reject invalid target actors."

$fireballBody = Get-FunctionBody `
  "void\s+ADBAZodiacCharacterBase::ServerCastLobbyFireball_Implementation\s*\(\s*FVector_NetQuantizeNormal\s+AimDirection\s*\)\s*\{" `
  "`nvoid\s+ADBAZodiacCharacterBase::ServerCastLobbyFireballAtTarget_Implementation" `
  "ServerCastLobbyFireball_Implementation"
$fireballTargetBody = Get-FunctionBody `
  "void\s+ADBAZodiacCharacterBase::ServerCastLobbyFireballAtTarget_Implementation\s*\(\s*AActor\*\s+TargetActor,\s*FVector_NetQuantizeNormal\s+FallbackAimDirection\s*\)\s*\{" `
  "`nvoid\s+ADBAZodiacCharacterBase::ServerCastEquippedSkill_Implementation" `
  "ServerCastLobbyFireballAtTarget_Implementation"
$equippedBody = Get-FunctionBody `
  "void\s+ADBAZodiacCharacterBase::ServerCastEquippedSkill_Implementation\s*\(\s*int32\s+SkillSlot,\s*AActor\*\s+TargetActor,\s*FVector_NetQuantizeNormal\s+FallbackAimDirection\s*\)\s*\{" `
  "`nvoid\s+ADBAZodiacCharacterBase::CastLobbyFireballInternal" `
  "ServerCastEquippedSkill_Implementation"

Assert-GuardBeforeCast $fireballBody "ServerCastLobbyFireball_Implementation"
Assert-GuardBeforeCast $fireballTargetBody "ServerCastLobbyFireballAtTarget_Implementation"
Assert-GuardBeforeCast $equippedBody "ServerCastEquippedSkill_Implementation"

Assert-True ($fireballBody.Contains("ValidateServerEquippedSkillCast(1, nullptr)")) `
  "Expected lobby fireball server RPC to validate slot 1 without a target."
Assert-True ($fireballTargetBody.Contains("ValidateServerEquippedSkillCast(1, TargetActor)")) `
  "Expected targeted lobby fireball server RPC to validate slot 1 with target."
Assert-True ($equippedBody.Contains("ValidateServerEquippedSkillCast(SkillSlot, TargetActor)")) `
  "Expected equipped skill server RPC to validate requested slot and target."

Write-Host "PASS: Zodiac character server cast authority boundary" -ForegroundColor Green
