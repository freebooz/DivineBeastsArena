<#
Validates Zodiac character client-side skill entrypoints only send server cast RPCs from locally controlled pawns.
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

function Assert-NonLocalClientGuardBeforeServerRpc {
  param(
    [Parameter(Mandatory = $true)][string]$Body,
    [Parameter(Mandatory = $true)][string]$FunctionName
  )

  $guardIndex = $Body.IndexOf("!HasAuthority() && !IsLocallyControlled()")
  $returnIndex = $Body.IndexOf("return", [Math]::Max(0, $guardIndex))
  $serverRpcIndex = $Body.IndexOf("ServerCastEquippedSkill(")
  $internalCastIndex = $Body.IndexOf("CastEquippedSkillInternal")

  Assert-True ($guardIndex -ge 0) "Expected $FunctionName to reject non-local client proxies before sending ServerCastEquippedSkill."
  Assert-True ($returnIndex -gt $guardIndex) "Expected $FunctionName non-local client guard to return."
  Assert-True ($serverRpcIndex -gt $returnIndex) "Expected $FunctionName to send ServerCastEquippedSkill only after non-local client guard."
  Assert-True ($internalCastIndex -gt $serverRpcIndex) "Expected $FunctionName authority-side internal cast to remain after client RPC branch."
}

$castBody = Get-FunctionBody `
  "void\s+ADBAZodiacCharacterBase::CastEquippedSkill\s*\(\s*int32\s+SkillSlot\s*\)\s*\{" `
  "`nvoid\s+ADBAZodiacCharacterBase::CastEquippedSkillAtTarget" `
  "CastEquippedSkill"

$targetCastBody = Get-FunctionBody `
  "void\s+ADBAZodiacCharacterBase::CastEquippedSkillAtTarget\s*\(\s*int32\s+SkillSlot,\s*AActor\*\s+TargetActor\s*\)\s*\{" `
  "`nbool\s+ADBAZodiacCharacterBase::ValidateServerEquippedSkillCast" `
  "CastEquippedSkillAtTarget"

Assert-NonLocalClientGuardBeforeServerRpc $castBody "CastEquippedSkill"
Assert-NonLocalClientGuardBeforeServerRpc $targetCastBody "CastEquippedSkillAtTarget"

Write-Host "PASS: Zodiac character local skill RPC boundary" -ForegroundColor Green
