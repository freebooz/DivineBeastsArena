<#
Validates replicated ASC gameplay state can only be mutated by authority.
These functions remain BlueprintCallable C++ bridges for UI/config/debug usage,
but UltimateEnergy, ChainLevel, and ResonanceLevel writes must fail closed.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$ascCppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\GAS\DBAAbilitySystemComponent.cpp"
$ascHeaderPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\GAS\DBAAbilitySystemComponent.h"
$ascCpp = Get-Content -LiteralPath $ascCppPath -Encoding UTF8 -Raw
$ascHeader = Get-Content -LiteralPath $ascHeaderPath -Encoding UTF8 -Raw

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

Assert-True ($ascHeader.Contains("UFUNCTION(BlueprintCallable, Category = ""DBA|Ultimate"")")) `
  "Expected UltimateEnergy functions to remain BlueprintCallable C++ bridge entrypoints."
Assert-True ($ascHeader.Contains("UFUNCTION(BlueprintCallable, Category = ""DBA|Chain"")")) `
  "Expected ChainLevel functions to remain BlueprintCallable C++ bridge entrypoints."
Assert-True ($ascHeader.Contains("UFUNCTION(BlueprintCallable, Category = ""DBA|Resonance"")")) `
  "Expected ResonanceLevel functions to remain BlueprintCallable C++ bridge entrypoints."

$functions = @(
  @{
    Name = "AddUltimateEnergy"
    Start = "void\s+UDBAAbilitySystemComponent::AddUltimateEnergy\s*\("
    End = "`nbool\s+UDBAAbilitySystemComponent::ConsumeUltimateEnergy"
    Writes = @("UltimateEnergy = FMath::Clamp", "BroadcastUltimateEnergyChanged()")
  },
  @{
    Name = "ConsumeUltimateEnergy"
    Start = "bool\s+UDBAAbilitySystemComponent::ConsumeUltimateEnergy\s*\("
    End = "`nbool\s+UDBAAbilitySystemComponent::HasEnoughUltimateEnergy"
    Writes = @("UltimateEnergy = FMath::Clamp", "BroadcastUltimateEnergyChanged()")
  },
  @{
    Name = "AddChainLevel"
    Start = "void\s+UDBAAbilitySystemComponent::AddChainLevel\s*\("
    End = "`nvoid\s+UDBAAbilitySystemComponent::ResetChainLevel"
    Writes = @("ChainLevel = FMath::Clamp", "BroadcastChainLevelChanged()", "SetTimer(ChainResetTimerHandle")
  },
  @{
    Name = "ResetChainLevel"
    Start = "void\s+UDBAAbilitySystemComponent::ResetChainLevel\s*\("
    End = "`nbool\s+UDBAAbilitySystemComponent::ShouldTriggerChainFinisher"
    Writes = @("ChainLevel = 0", "BroadcastChainLevelChanged()", "ClearTimer(ChainResetTimerHandle")
  },
  @{
    Name = "SetResonanceLevel"
    Start = "void\s+UDBAAbilitySystemComponent::SetResonanceLevel\s*\("
    End = "`nbool\s+UDBAAbilitySystemComponent::CanActivateAbility"
    Writes = @("ResonanceLevel = FMath::Clamp", "BroadcastResonanceLevelChanged()")
  }
)

foreach ($function in $functions) {
  $body = Get-FunctionBody $function.Start $function.End $function.Name
  Assert-True ($body -match "if\s*\(\s*GetOwnerRole\(\)\s*!=\s*ROLE_Authority") `
    "Expected $($function.Name) to explicitly fail closed for non-authority callers."
  Assert-True ($body.Contains("return")) `
    "Expected $($function.Name) authority guard to return before state mutation."

  $guardIndex = $body.IndexOf("if (GetOwnerRole() != ROLE_Authority)")
  foreach ($write in $function.Writes) {
    $writeIndex = $body.IndexOf($write)
    Assert-True ($writeIndex -gt $guardIndex) `
      "Expected $($function.Name) to guard authority before: $write"
  }
}

Write-Host "PASS: AbilitySystem state authority boundary contract" -ForegroundColor Green
