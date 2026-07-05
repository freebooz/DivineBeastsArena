<#
Validates Zodiac character state bridge entrypoints remain server-authoritative.
UltimateEnergy and ChainLevel bridge methods must delegate to ASC, while cooldown
cache writes remain guarded fallback state for spectator/UI compatibility.
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

foreach ($entrypoint in @("SetUltimateEnergy", "AddUltimateEnergy", "AddChainLevel", "ResetChainLevel")) {
  Assert-True ($header -match "UFUNCTION\(BlueprintCallable,\s*Category\s*=\s*""DBA\|Character\|Attribute""\)[\s\S]{0,120}void\s+$entrypoint\s*\(") `
    "Expected $entrypoint to remain a BlueprintCallable C++ bridge entrypoint."
}

$functions = @(
  @{
    Name = "SetUltimateEnergy"
    Start = "void\s+ADBAZodiacCharacterBase::SetUltimateEnergy\s*\("
    End = "`nvoid\s+ADBAZodiacCharacterBase::AddUltimateEnergy"
    Required = @("GetDBAAbilitySystemComponent()", "ASC->AddUltimateEnergy", "ASC->ConsumeUltimateEnergy")
    Forbidden = @("UltimateEnergy = FMath::Clamp")
  },
  @{
    Name = "AddUltimateEnergy"
    Start = "void\s+ADBAZodiacCharacterBase::AddUltimateEnergy\s*\("
    End = "`nbool\s+ADBAZodiacCharacterBase::IsUltimateReady"
    Required = @("GetDBAAbilitySystemComponent()", "ASC->AddUltimateEnergy(Delta)")
    Forbidden = @("UltimateEnergy = FMath::Clamp")
  },
  @{
    Name = "AddChainLevel"
    Start = "void\s+ADBAZodiacCharacterBase::AddChainLevel\s*\("
    End = "`nvoid\s+ADBAZodiacCharacterBase::ResetChainLevel"
    Required = @("GetDBAAbilitySystemComponent()", "ASC->AddChainLevel(Delta)")
    Forbidden = @("ChainLevel = FMath::Clamp")
  },
  @{
    Name = "ResetChainLevel"
    Start = "void\s+ADBAZodiacCharacterBase::ResetChainLevel\s*\("
    End = "`nvoid\s+ADBAZodiacCharacterBase::PlayAttackAnimation"
    Required = @("GetDBAAbilitySystemComponent()", "ASC->ResetChainLevel()")
    Forbidden = @("ChainLevel = 0")
  },
  @{
    Name = "UpdateSkillCooldowns"
    Start = "void\s+ADBAZodiacCharacterBase::UpdateSkillCooldowns\s*\("
    End = "`nvoid\s+ADBAZodiacCharacterBase::OnRep_SkillCooldowns"
    Required = @("SkillCooldowns = NewCooldowns", "SkillMaxCooldowns.Add", "OnSkillCooldownsChanged.Broadcast")
    Forbidden = @()
  }
)

foreach ($function in $functions) {
  $body = Get-FunctionBody $function.Start $function.End $function.Name
  Assert-True ($body.Contains("if (!HasAuthority())")) `
    "Expected $($function.Name) to explicitly fail closed for non-authority callers."
  Assert-True ($body.Contains("return")) `
    "Expected $($function.Name) authority guard to return before fallback state mutation."

  $guardIndex = $body.IndexOf("if (!HasAuthority())")
  foreach ($required in $function.Required) {
    $requiredIndex = $body.IndexOf($required)
    Assert-True ($requiredIndex -gt $guardIndex) `
      "Expected $($function.Name) to guard authority before: $required"
  }

  foreach ($forbidden in $function.Forbidden) {
    Assert-True (-not $body.Contains($forbidden)) `
      "Expected $($function.Name) to avoid direct Character state write: $forbidden"
  }
}

Write-Host "PASS: Zodiac character fallback state authority boundary contract" -ForegroundColor Green
