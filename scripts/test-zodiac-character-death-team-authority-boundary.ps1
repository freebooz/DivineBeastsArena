<#
Validates Zodiac character death and TeamId replicated state can only be
mutated by authority. These BlueprintCallable methods are kept as C++ bridge
entrypoints, but runtime state writes must fail closed before mutation.
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

foreach ($entrypoint in @("OnDeath", "OnRevive")) {
  Assert-True ($header -match "UFUNCTION\(BlueprintCallable,\s*Category\s*=\s*""DBA\|Character\|Death""\)[\s\S]{0,120}void\s+$entrypoint\s*\(") `
    "Expected $entrypoint to remain a BlueprintCallable C++ bridge entrypoint."
}

Assert-True ($header -match "UFUNCTION\(BlueprintCallable,\s*Category\s*=\s*""DBA\|Character\|Team""\)[\s\S]{0,120}void\s+SetTeamID\s*\(") `
  "Expected SetTeamID to remain a BlueprintCallable C++ bridge entrypoint."

$functions = @(
  @{
    Name = "OnDeath"
    Start = "void\s+ADBAZodiacCharacterBase::OnDeath\s*\("
    End = "`nvoid\s+ADBAZodiacCharacterBase::OnRevive"
    Writes = @("DeathState = EDADeathState::Dying", "PlayDeathAnimation", "DeathState = EDADeathState::Dead")
  },
  @{
    Name = "OnRevive"
    Start = "void\s+ADBAZodiacCharacterBase::OnRevive\s*\("
    End = "`nvoid\s+ADBAZodiacCharacterBase::UpdateSkillCooldowns"
    Writes = @("DeathState = EDADeathState::Alive")
  },
  @{
    Name = "SetTeamID"
    Start = "void\s+ADBAZodiacCharacterBase::SetTeamID\s*\("
    End = "`nbool\s+ADBAZodiacCharacterBase::IsTeammate"
    Writes = @("TeamID = FMath::Max")
  }
)

foreach ($function in $functions) {
  $body = Get-FunctionBody $function.Start $function.End $function.Name
  Assert-True ($body.Contains("if (!HasAuthority())")) `
    "Expected $($function.Name) to explicitly fail closed for non-authority callers."
  Assert-True ($body.Contains("return;")) `
    "Expected $($function.Name) authority guard to return before replicated state mutation."

  $guardIndex = $body.IndexOf("if (!HasAuthority())")
  foreach ($write in $function.Writes) {
    $writeIndex = $body.IndexOf($write)
    Assert-True ($writeIndex -gt $guardIndex) `
      "Expected $($function.Name) to guard authority before: $write"
  }
}

Write-Host "PASS: Zodiac character death/team authority boundary contract" -ForegroundColor Green
