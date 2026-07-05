<#
Validates that the Angular Admin match detail page can consume and display
settlement reward payloads returned by /api/admin/matches/{matchId}.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

function Assert-FileContains {
  param(
    [Parameter(Mandatory = $true)][string]$RelativePath,
    [Parameter(Mandatory = $true)][string[]]$RequiredTokens
  )

  $fullPath = Join-Path $repoRoot $RelativePath
  if (-not (Test-Path -LiteralPath $fullPath)) {
    throw "Required file is missing: $RelativePath"
  }

  $content = Get-Content -Raw -Encoding UTF8 -LiteralPath $fullPath
  $missing = @($RequiredTokens | Where-Object { -not $content.Contains($_) })
  if ($missing.Count -gt 0) {
    throw "$RelativePath is missing Admin match reward display contract symbols: $($missing -join ', ')"
  }
}

Assert-FileContains "DBA_GameAdmin\src\app\core\models.ts" @(
  "rewards: Record<string, unknown>",
  "winnerTeam?: string | null",
  "winnerTeam?: string | null",
  "teamDistribution?: Record<string, number>",
  "export interface MatchPlayerItem"
)

Assert-FileContains "DBA_GameAdmin\src\app\pages\admin-pages.ts" @(
  "formatResultSummary",
  "formatResultSummary(match)",
  "match.winnerTeam?.trim()",
  "winner_team",
  "schema",
  "formatRewards",
  "formatRewards(player.rewards)",
  "Object.entries(rewards ?? {})",
  "key}: ${String(value)}",
  "match.id",
  "match.sessionId",
  "match.durationSeconds",
  "match.createdAt",
  "prettyResult"
  "team-outcome-winner",
  "team-outcome-distribution",
  "formatTeamOutcome(match)",
  "formatTeamDistribution(match.players, match.teamDistribution)",
  "teamDistribution?: Record<string, number> | null",
  "extractWinnerTeam(match.resultJson)",
  "player.team?.trim()"
)

Assert-FileContains "DBA_GameBackend\docs\api.md" @(
  "Admin match result diagnostics",
  "resultJson",
  "winnerTeam",
  "winner_team",
  "Team outcome summary",
  'winnerTeam`',
  'teamDistribution`',
  "rewards"
)

Assert-FileContains "DBA_GameBackend\Game.Api.Tests\AdminMatchEndpointsTests.cs" @(
  "RuntimeMatchResults_CanBeReadFromAdminMatchDetailsForOperationsDiagnostics",
  "runtime-token-admin-query",
  "match-result-runtime-admin-query",
  '"schema":"runtime-admin-query-test"',
  '"/runtime/matches/results"',
  '"/api/admin/matches/{matchResultId}"',
  'Assert.Equal("blue", adminResponse.Data.WinnerTeam)',
  'Assert.Equal(1, adminResponse.Data.TeamDistribution["blue"])',
  'Assert.Equal(7, GetRewardInt(player.Rewards, "coin"))'
)

Write-Host "PASS: Admin match reward display contract" -ForegroundColor Green
