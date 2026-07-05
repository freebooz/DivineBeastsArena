<#
Validates GAS target enemy checks resolve TeamId through C++ runtime state.
Blueprint interfaces may expose presentation/configuration hooks, but they must
not decide authority target hostility for ability activation.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$ascCppRelativePath = "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"
$ascCppPath = Join-Path -Path $repoRoot -ChildPath $ascCppRelativePath

if (-not (Test-Path -LiteralPath $ascCppPath)) {
  throw "Required AbilitySystem source file is missing: $ascCppPath"
}

$ascCpp = Get-Content -LiteralPath $ascCppPath -Encoding UTF8 -Raw

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

  $match = [regex]::Match($ascCpp, $StartPattern)
  Assert-True $match.Success "Expected $FunctionName implementation."

  $remaining = $ascCpp.Substring($match.Index + $match.Length)
  $nextMatch = [regex]::Match($remaining, $EndPattern)
  Assert-True $nextMatch.Success "Expected end boundary after $FunctionName implementation."

  return $ascCpp.Substring($match.Index, $match.Length + $nextMatch.Index)
}

Assert-True ($ascCpp -match "bool\s+ResolveActorTeamIdForAbilityTargeting\s*\(\s*const\s+AActor\*\s+Actor\s*,\s*int32&\s+OutTeamId\s*\)") `
  "Expected C++ TeamId resolver for GAS ability target validation."
Assert-True (-not ($ascCpp -match "Execute_GetTeamId")) `
  "GAS target validation must not execute Blueprint TeamAgent GetTeamId."

$resolverBody = Get-FunctionBody `
  "bool\s+ResolveActorTeamIdForAbilityTargeting\s*\(" `
  "`nbool\s+UDBAAbilitySystemComponent::" `
  "ResolveActorTeamIdForAbilityTargeting"

Assert-True ($resolverBody.Contains("Cast<ADBAZodiacCharacterBase>(Actor)")) `
  "Expected TeamId resolver to read ADBAZodiacCharacterBase C++ replicated TeamID."
Assert-True ($resolverBody.Contains("GetTeamID()")) `
  "Expected TeamId resolver to use the C++ GetTeamID accessor."
Assert-True ($resolverBody -match "ResolvedTeamId\s*>\s*0") `
  "Expected TeamId resolver to reject non-positive or neutral TeamId values."

$targetBody = Get-FunctionBody `
  "bool\s+UDBAAbilitySystemComponent::IsValidTarget\s*\(\s*AActor\*\s+Target\s*,\s*bool\s+bRequireEnemy\s*\)\s+const\s*\{" `
  "`nvoid\s+UDBAAbilitySystemComponent::TriggerGameplayCue" `
  "IsValidTarget"

Assert-True ($targetBody.Contains("ResolveActorTeamIdForAbilityTargeting(SourceActor, SourceTeamId)")) `
  "Expected IsValidTarget to resolve source TeamId through the C++ resolver."
Assert-True ($targetBody.Contains("ResolveActorTeamIdForAbilityTargeting(Target, TargetTeamId)")) `
  "Expected IsValidTarget to resolve target TeamId through the C++ resolver."
Assert-True ($targetBody -match "SourceTeamId\s*!=\s*TargetTeamId") `
  "Expected enemy validation to compare C++ resolved TeamIds."

Write-Host "PASS: AbilitySystem target TeamId C++ boundary" -ForegroundColor Green
