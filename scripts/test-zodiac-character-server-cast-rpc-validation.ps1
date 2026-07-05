<#
Validates Zodiac character server cast RPC declarations and _Validate implementations reject invalid casts before implementation.
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

function Assert-HeaderRpcHasValidation {
  param(
    [Parameter(Mandatory = $true)][string]$FunctionName
  )

  $pattern = "UFUNCTION\s*\(\s*Server\s*,\s*Reliable\s*,\s*WithValidation\s*\)\s*`r?`n\s*void\s+$FunctionName\s*\("
  Assert-True ($header -match $pattern) "Expected $FunctionName to be declared as Server, Reliable, WithValidation."
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

function Assert-ValidateCallsGuard {
  param(
    [Parameter(Mandatory = $true)][string]$Body,
    [Parameter(Mandatory = $true)][string]$FunctionName,
    [Parameter(Mandatory = $true)][string]$ExpectedGuardCall
  )

  Assert-True ($Body.Contains("return $ExpectedGuardCall;")) "Expected $FunctionName to return $ExpectedGuardCall."
  Assert-True (-not $Body.Contains("CastEquippedSkillInternal")) "Expected $FunctionName to validate only, not perform the cast."
}

Assert-HeaderRpcHasValidation "ServerCastLobbyFireball"
Assert-HeaderRpcHasValidation "ServerCastLobbyFireballAtTarget"
Assert-HeaderRpcHasValidation "ServerCastEquippedSkill"

$fireballValidate = Get-FunctionBody `
  "bool\s+ADBAZodiacCharacterBase::ServerCastLobbyFireball_Validate\s*\(\s*FVector_NetQuantizeNormal\s+AimDirection\s*\)\s*\{" `
  "`nvoid\s+ADBAZodiacCharacterBase::ServerCastLobbyFireball_Implementation" `
  "ServerCastLobbyFireball_Validate"
$fireballTargetValidate = Get-FunctionBody `
  "bool\s+ADBAZodiacCharacterBase::ServerCastLobbyFireballAtTarget_Validate\s*\(\s*AActor\*\s+TargetActor,\s*FVector_NetQuantizeNormal\s+FallbackAimDirection\s*\)\s*\{" `
  "`nvoid\s+ADBAZodiacCharacterBase::ServerCastLobbyFireballAtTarget_Implementation" `
  "ServerCastLobbyFireballAtTarget_Validate"
$equippedValidate = Get-FunctionBody `
  "bool\s+ADBAZodiacCharacterBase::ServerCastEquippedSkill_Validate\s*\(\s*int32\s+SkillSlot,\s*AActor\*\s+TargetActor,\s*FVector_NetQuantizeNormal\s+FallbackAimDirection\s*\)\s*\{" `
  "`nvoid\s+ADBAZodiacCharacterBase::ServerCastEquippedSkill_Implementation" `
  "ServerCastEquippedSkill_Validate"

Assert-ValidateCallsGuard $fireballValidate "ServerCastLobbyFireball_Validate" "ValidateServerEquippedSkillCast(1, nullptr)"
Assert-ValidateCallsGuard $fireballTargetValidate "ServerCastLobbyFireballAtTarget_Validate" "ValidateServerEquippedSkillCast(1, TargetActor)"
Assert-ValidateCallsGuard $equippedValidate "ServerCastEquippedSkill_Validate" "ValidateServerEquippedSkillCast(SkillSlot, TargetActor)"

Write-Host "PASS: Zodiac character server cast RPC validation contract" -ForegroundColor Green
