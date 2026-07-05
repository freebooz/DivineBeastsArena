<#
Validates Zodiac character cooldown lookup resolves SkillId through C++ runtime
skill specs and reads the replicated cooldown cache with the same 0-based slot
indexing used by the Arena AbilityBar.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"
$interfacePath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Character\IDBACharacterRef.h"

$cpp = Get-Content -LiteralPath $cppPath -Encoding UTF8 -Raw
$interfaceHeader = Get-Content -LiteralPath $interfacePath -Encoding UTF8 -Raw

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

$cooldownQueryBody = Get-FunctionBody `
  "bool\s+ADBAZodiacCharacterBase::IsAbilityOnCooldown\s*\(\s*FName\s+SkillId\s*\)\s*const\s*\{" `
  "`nbool\s+ADBAZodiacCharacterBase::HasEnoughEnergy" `
  "IsAbilityOnCooldown"

Assert-True ($interfaceHeader -match "virtual\s+bool\s+IsAbilityOnCooldown\s*\(\s*FName\s+SkillId\s*\)\s*const\s*=\s*0") `
  "Expected IDBACharacterRef to keep the cooldown query as a C++ interface contract."
Assert-True ($cooldownQueryBody.Contains("SkillId.IsNone()")) `
  "Expected IsAbilityOnCooldown to reject an empty SkillId safely."
Assert-True ($cooldownQueryBody.Contains("GetPlayableSkillSpecs()")) `
  "Expected IsAbilityOnCooldown to resolve SkillId through playable skill runtime specs."
Assert-True ($cooldownQueryBody -match 'const\s+FDBAPlayableSkillRuntimeSpec&\s+SkillSpec') `
  "Expected IsAbilityOnCooldown to inspect FDBAPlayableSkillRuntimeSpec entries."
Assert-True ($cooldownQueryBody -match 'SkillSpec\.SkillId\s*==\s*SkillId') `
  "Expected IsAbilityOnCooldown to match the requested SkillId."
Assert-True ($cooldownQueryBody.Contains("const int32 CooldownArrayIndex = SkillSpec.SkillSlot - 1;")) `
  "Expected IsAbilityOnCooldown to convert 1-based SkillSlot to 0-based CooldownArrayIndex."
Assert-True ($cooldownQueryBody.Contains("SkillCooldowns.IsValidIndex(CooldownArrayIndex)")) `
  "Expected IsAbilityOnCooldown to guard replicated cooldown array access."
Assert-True ($cooldownQueryBody -match 'SkillCooldowns\[CooldownArrayIndex\]\s*>\s*0\.0f') `
  "Expected IsAbilityOnCooldown to report active cooldowns from SkillCooldowns."

Write-Host "PASS: Zodiac character ability cooldown query contract" -ForegroundColor Green
