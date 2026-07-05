<#
Validates replicated PlayerState match stats can only be mutated by authority.
These BlueprintCallable methods are C++ gameplay/stat bridge entrypoints; clients
must consume replicated values for HUD/scoreboard and never write settlement data.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path -Path $PSScriptRoot -ChildPath "..")).ProviderPath
$cppPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\Player\DBAPlayerState.cpp"
$headerPath = Join-Path -Path $repoRoot -ChildPath "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\Player\DBAPlayerState.h"
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

$blueprintEntryPoints = @(
  "RecordKill",
  "RecordDeath",
  "RecordAssist",
  "AddMatchScore",
  "AddMatchExpDelta",
  "SetMatchResult",
  "SetMatchTeamId"
)

foreach ($entrypoint in $blueprintEntryPoints) {
  Assert-True ($header -match "UFUNCTION\(BlueprintCallable,\s*Category\s*=\s*""DBA\|MatchStats""\)[\s\S]{0,120}void\s+$entrypoint\s*\(") `
    "Expected $entrypoint to remain a BlueprintCallable C++ match-stat bridge entrypoint."
}

$functions = @(
  @{
    Name = "RecordKill"
    Start = "void\s+ADBAPlayerState::RecordKill\s*\("
    End = "`nvoid\s+ADBAPlayerState::RecordDeath"
    Writes = @("MatchKills +=")
  },
  @{
    Name = "RecordDeath"
    Start = "void\s+ADBAPlayerState::RecordDeath\s*\("
    End = "`nvoid\s+ADBAPlayerState::RecordAssist"
    Writes = @("MatchDeaths +=")
  },
  @{
    Name = "RecordAssist"
    Start = "void\s+ADBAPlayerState::RecordAssist\s*\("
    End = "`nvoid\s+ADBAPlayerState::AddMatchScore"
    Writes = @("MatchAssists +=")
  },
  @{
    Name = "AddMatchScore"
    Start = "void\s+ADBAPlayerState::AddMatchScore\s*\("
    End = "`nvoid\s+ADBAPlayerState::AddMatchExpDelta"
    Writes = @("MatchScore =")
  },
  @{
    Name = "AddMatchExpDelta"
    Start = "void\s+ADBAPlayerState::AddMatchExpDelta\s*\("
    End = "`nvoid\s+ADBAPlayerState::SetMatchResult"
    Writes = @("MatchExpDelta =")
  },
  @{
    Name = "SetMatchResult"
    Start = "void\s+ADBAPlayerState::SetMatchResult\s*\("
    End = "`nvoid\s+ADBAPlayerState::SetMatchTeamId"
    Writes = @("MatchResult =")
  },
  @{
    Name = "SetMatchTeamId"
    Start = "void\s+ADBAPlayerState::SetMatchTeamId\s*\("
    End = "`nFDBA_GameBackendRuntimePlayerResult\s+ADBAPlayerState::BuildRuntimePlayerResult"
    Writes = @("MatchTeamId =")
  }
)

foreach ($function in $functions) {
  $body = Get-FunctionBody $function.Start $function.End $function.Name
  Assert-True ($body.Contains("if (!HasAuthority())")) `
    "Expected $($function.Name) to explicitly fail closed for non-authority callers."
  Assert-True ($body.Contains("return")) `
    "Expected $($function.Name) authority guard to return before match-stat mutation."

  $guardIndex = $body.IndexOf("if (!HasAuthority())")
  foreach ($write in $function.Writes) {
    $writeIndex = $body.IndexOf($write)
    Assert-True ($writeIndex -gt $guardIndex) `
      "Expected $($function.Name) to guard authority before: $write"
  }
}

Write-Host "PASS: PlayerState match stats authority boundary contract" -ForegroundColor Green
