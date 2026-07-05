<#
Validates Zodiac character legacy fallback skill casts write cooldowns using the same 0-based array indexing consumed by GAS and AbilityBar UI.
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

$internalCastBody = Get-FunctionBody `
  "void\s+ADBAZodiacCharacterBase::CastEquippedSkillInternal\s*\(\s*int32\s+SkillSlot,\s*const\s+FVector&\s+AimDirection,\s*AActor\*\s+TargetActor\s*\)\s*\{" `
  "`nvoid\s+ADBAZodiacCharacterBase::MulticastPlayLobbySkillCastFeedback_Implementation" `
  "CastEquippedSkillInternal"

Assert-True ($internalCastBody.Contains("const int32 CooldownArrayIndex = SkillSlot - 1;")) `
  "Expected legacy skill cooldown path to convert 1-based SkillSlot to a 0-based CooldownArrayIndex."
Assert-True ($internalCastBody -match "SkillCooldowns\.IsValidIndex\(CooldownArrayIndex\)\s*&&\s*SkillCooldowns\[CooldownArrayIndex\]\s*>\s*0\.0f") `
  "Expected legacy skill cooldown check to read SkillCooldowns by CooldownArrayIndex."
Assert-True ($internalCastBody.Contains("SkillCooldowns[CooldownArrayIndex] = Spec.Cooldown;")) `
  "Expected legacy skill cooldown write to use CooldownArrayIndex."
Assert-True ($internalCastBody.Contains("SkillMaxCooldowns[CooldownArrayIndex] = Spec.Cooldown;")) `
  "Expected legacy max cooldown write to use CooldownArrayIndex."
Assert-True ($internalCastBody.Contains("OnSkillCooldownsChanged.Broadcast(SkillCooldowns);")) `
  "Expected legacy cooldown writes to notify local HUD listeners immediately."
Assert-True (-not $internalCastBody.Contains("SkillCooldowns[SkillSlot] = Spec.Cooldown;")) `
  "Expected legacy skill cooldown writes to avoid 1-based SkillSlot indexing."
Assert-True (-not $internalCastBody.Contains("SkillMaxCooldowns[SkillSlot] = Spec.Cooldown;")) `
  "Expected legacy max cooldown writes to avoid 1-based SkillSlot indexing."

Write-Host "PASS: Zodiac character legacy cooldown indexing contract" -ForegroundColor Green
