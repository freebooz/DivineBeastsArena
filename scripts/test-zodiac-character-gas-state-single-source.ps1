<#
Validates that ADBAZodiacCharacterBase does not act as an authoritative writer
for GAS combat state after ASC is available.

The current project-level rule is:
- UltimateEnergy, ChainLevel, and ResonanceLevel authoritative mutations live in
  UDBAAbilitySystemComponent.
- Character bridge methods may remain BlueprintCallable for compatibility, but
  they must delegate writes to ASC instead of mutating replicated Character
  fallback fields.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$characterCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Character\DBAZodiacCharacterBase.cpp"
$characterCpp = Get-Content -LiteralPath $characterCppPath -Encoding UTF8 -Raw

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
    [Parameter(Mandatory = $true)][string]$FunctionName,
    [Parameter(Mandatory = $true)][string]$StartPattern,
    [Parameter(Mandatory = $true)][string]$EndPattern
  )

  $match = [regex]::Match($characterCpp, $StartPattern)
  Assert-True $match.Success "Expected $FunctionName implementation."

  $remaining = $characterCpp.Substring($match.Index + $match.Length)
  $nextMatch = [regex]::Match($remaining, $EndPattern)
  Assert-True $nextMatch.Success "Expected end boundary after $FunctionName implementation."

  return $characterCpp.Substring($match.Index, $match.Length + $nextMatch.Index)
}

$setUltimateBody = Get-FunctionBody `
  -FunctionName "SetUltimateEnergy" `
  -StartPattern "void\s+ADBAZodiacCharacterBase::SetUltimateEnergy\s*\(" `
  -EndPattern "`nvoid\s+ADBAZodiacCharacterBase::AddUltimateEnergy"
$addUltimateBody = Get-FunctionBody `
  -FunctionName "AddUltimateEnergy" `
  -StartPattern "void\s+ADBAZodiacCharacterBase::AddUltimateEnergy\s*\(" `
  -EndPattern "`nbool\s+ADBAZodiacCharacterBase::IsUltimateReady"
$isUltimateReadyBody = Get-FunctionBody `
  -FunctionName "IsUltimateReady" `
  -StartPattern "bool\s+ADBAZodiacCharacterBase::IsUltimateReady\s*\(" `
  -EndPattern "`nvoid\s+ADBAZodiacCharacterBase::AddChainLevel"
$addChainBody = Get-FunctionBody `
  -FunctionName "AddChainLevel" `
  -StartPattern "void\s+ADBAZodiacCharacterBase::AddChainLevel\s*\(" `
  -EndPattern "`nvoid\s+ADBAZodiacCharacterBase::ResetChainLevel"
$resetChainBody = Get-FunctionBody `
  -FunctionName "ResetChainLevel" `
  -StartPattern "void\s+ADBAZodiacCharacterBase::ResetChainLevel\s*\(" `
  -EndPattern "`nvoid\s+ADBAZodiacCharacterBase::PlayAttackAnimation"
$spectatorBody = Get-FunctionBody `
  -FunctionName "GetSpectatorData" `
  -StartPattern "void\s+ADBAZodiacCharacterBase::GetSpectatorData\s*\(" `
  -EndPattern "`nvoid\s+ADBAZodiacCharacterBase::SetTeamID"

Assert-True ($setUltimateBody.Contains("GetDBAAbilitySystemComponent()")) `
  "Expected SetUltimateEnergy to resolve DBA ASC."
Assert-True ($setUltimateBody.Contains("AddUltimateEnergy(") -or $setUltimateBody.Contains("ConsumeUltimateEnergy(")) `
  "Expected SetUltimateEnergy to delegate UltimateEnergy mutation to ASC."
Assert-True (-not ($setUltimateBody -match "(?m)^\s*UltimateEnergy\s*=")) `
  "SetUltimateEnergy must not write Character UltimateEnergy directly."

Assert-True ($addUltimateBody.Contains("GetDBAAbilitySystemComponent()")) `
  "Expected AddUltimateEnergy to resolve DBA ASC."
Assert-True ($addUltimateBody.Contains("ASC->AddUltimateEnergy(Delta)")) `
  "Expected AddUltimateEnergy to delegate to ASC->AddUltimateEnergy."
Assert-True (-not ($addUltimateBody -match "(?m)^\s*UltimateEnergy\s*=")) `
  "AddUltimateEnergy must not write Character UltimateEnergy directly."

Assert-True ($isUltimateReadyBody.Contains("GetUltimateEnergy()")) `
  "Expected IsUltimateReady to use GetUltimateEnergy so it reads ASC first."
Assert-True (-not ($isUltimateReadyBody -match "return\s+UltimateEnergy\s*>=")) `
  "IsUltimateReady must not read Character UltimateEnergy directly."

Assert-True ($addChainBody.Contains("GetDBAAbilitySystemComponent()")) `
  "Expected AddChainLevel to resolve DBA ASC."
Assert-True ($addChainBody.Contains("ASC->AddChainLevel(Delta)")) `
  "Expected AddChainLevel to delegate to ASC->AddChainLevel."
Assert-True (-not ($addChainBody -match "(?m)^\s*ChainLevel\s*=")) `
  "AddChainLevel must not write Character ChainLevel directly."

Assert-True ($resetChainBody.Contains("GetDBAAbilitySystemComponent()")) `
  "Expected ResetChainLevel to resolve DBA ASC."
Assert-True ($resetChainBody.Contains("ASC->ResetChainLevel()")) `
  "Expected ResetChainLevel to delegate to ASC->ResetChainLevel."
Assert-True (-not ($resetChainBody -match "(?m)^\s*ChainLevel\s*=")) `
  "ResetChainLevel must not write Character ChainLevel directly."

Assert-True ($spectatorBody.Contains("GetUltimateEnergy()")) `
  "Expected spectator data to read UltimateEnergy through Character getter."
Assert-True (-not ($spectatorBody.Contains("OutData.UltimateEnergy = UltimateEnergy"))) `
  "Spectator data must not read Character UltimateEnergy directly."

Write-Host "PASS: Zodiac character GAS state single-source contract" -ForegroundColor Green
