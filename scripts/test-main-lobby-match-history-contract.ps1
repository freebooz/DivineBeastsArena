<#
Validates the Main Lobby match-history summary contract.

The Lobby UI should consume a structured recent-match summary from its
controller instead of parsing backend match-history JSON in Blueprint.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")

function New-CodePointText {
    param(
        [Parameter(Mandatory = $true)][int[]]$CodePoints
    )

    return -join ($CodePoints | ForEach-Object { [char]$_ })
}

function New-TestEqualTextToken {
    param(
        [Parameter(Mandatory = $true)][string]$Message,
        [Parameter(Mandatory = $true)][string]$Expression
    )

    return 'TestEqual(TEXT("' + $Message + '"), ' + $Expression
}

$recentKillsAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x51FB, 0x6740, 0x6570, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.Kills, 8)'
$recentDeathsAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x6B7B, 0x4EA1, 0x6570, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.Deaths, 1)'
$recentAssistsAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x52A9, 0x653B, 0x6570, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.Assists, 6)'
$recentDurationAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x6301, 0x7EED, 0x65F6, 0x95F4, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.DurationSeconds, 420)'
$recentCombatSummaryAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x6218, 0x6597, 0x6458, 0x8981, 0x5E94, 0x88AB, 0x683C, 0x5F0F, 0x5316)) 'Summary.CombatSummary, FString(TEXT("KDA 8/1/6 / 07:00")))'
$recentPlayedAtAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x6E38, 0x73A9, 0x65F6, 0x95F4, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.PlayedAtUtc, FString(TEXT("2026-07-01T02:00:00Z")))'
$recentExpDeltaAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x7ECF, 0x9A8C, 0x53D8, 0x5316, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.ExpDelta, static_cast<int64>(1200))'
$recentCoinRewardAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x91D1, 0x5E01, 0x5956, 0x52B1, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.CoinReward, static_cast<int64>(12))'
$recentHonorRewardAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x8363, 0x8A89, 0x5956, 0x52B1, 0x5E94, 0x88AB, 0x89E3, 0x6790)) 'Summary.HonorReward, static_cast<int64>(5))'
$recentRewardSummaryAssertionText = New-TestEqualTextToken (New-CodePointText @(0x6700, 0x8FD1, 0x5956, 0x52B1, 0x6458, 0x8981, 0x5E94, 0x5305, 0x542B, 0x5168, 0x90E8, 0x6570, 0x503C, 0x5956, 0x52B1)) 'Summary.RewardSummary, FString(TEXT("coin +12 / gem +2 / honor +5")))'

function Assert-FileContains {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][string[]]$RequiredSymbols
    )

    $fullPath = Join-Path $repoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $fullPath)) {
        throw "Required file is missing: $RelativePath"
    }

    $content = Get-Content -Raw -Encoding UTF8 $fullPath
    $missing = @($RequiredSymbols | Where-Object { $content -notmatch [regex]::Escape($_) })
    if ($missing.Count -gt 0) {
        throw "$RelativePath is missing contract symbols: $($missing -join ', ')"
    }
}

function Assert-FileMatches {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][string[]]$RequiredPatterns
    )

    $fullPath = Join-Path $repoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $fullPath)) {
        throw "Required file is missing: $RelativePath"
    }

    $content = Get-Content -Raw -Encoding UTF8 $fullPath
    $missing = @($RequiredPatterns | Where-Object { $content -notmatch $_ })
    if ($missing.Count -gt 0) {
        throw "$RelativePath is missing contract patterns: $($missing -join ', ')"
    }
}

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Lobby\UDBAMainLobbyWidgetController.h" @(
    "FDBALobbyRecentMatchSummary",
    "bool bHasMatch",
    "FString WinnerTeam",
    "int32 Kills",
    "int32 Deaths",
    "int32 Assists",
    "int32 DurationSeconds",
    "FString PlayedAtUtc",
    "int64 ExpDelta",
    "int64 CoinReward",
    "int64 HonorReward",
    "FString RewardSummary",
    "FString CombatSummary",
    "RefreshMatchHistory",
    "UpdateMatchHistoryFromJson",
    "GetRecentMatchSummary",
    "OnMatchHistoryUpdated",
    "OnRecentMatchSummaryUpdated",
    "HandleGetMatchHistoryResponse"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\UDBAMainLobbyWidgetController.cpp" @(
    "void UDBAMainLobbyWidgetController::InitializeBackendLobby()",
    "RefreshMatchHistory();",
    "void UDBAMainLobbyWidgetController::RefreshMatchHistory()",
    "PlayerService->GetMyMatches(Callback)",
    "bool UDBAMainLobbyWidgetController::UpdateMatchHistoryFromJson",
    "UDBA_GameBackendPlayerService::TryParseMatchHistoryData",
    "RecentMatchSummary.bHasMatch = true",
    "RecentMatchSummary.WinnerTeam = Match.WinnerTeam",
    "RecentMatchSummary.ExpDelta = Match.ExpDelta",
    "RecentMatchSummary.Kills = Match.Kills",
    "RecentMatchSummary.Deaths = Match.Deaths",
    "RecentMatchSummary.Assists = Match.Assists",
    "RecentMatchSummary.DurationSeconds = Match.DurationSeconds",
    "RecentMatchSummary.PlayedAtUtc = Match.PlayedAtUtc",
    "RecentMatchSummary.CoinReward = Match.Rewards.FindRef(TEXT(""coin""))",
    "RecentMatchSummary.HonorReward = Match.Rewards.FindRef(TEXT(""honor""))",
    "RecentMatchSummary.RewardSummary = BuildRecentMatchRewardSummary(Match.Rewards)",
    "RecentMatchSummary.CombatSummary = BuildRecentMatchCombatSummary(Match.Kills, Match.Deaths, Match.Assists, Match.DurationSeconds)",
    "OnRecentMatchSummaryUpdated.Broadcast(RecentMatchSummary)",
    "void UDBAMainLobbyWidgetController::HandleGetMatchHistoryResponse",
    "OnMatchHistoryUpdated.Broadcast(DataJson)",
    "void UDBAMainLobbyWidgetController::NotifyMatchFinishedClientView()",
    "TrackTelemetry(TEXT(""match_finished_client_view"")",
    "RefreshPlayerData();",
    "RefreshMatchHistory();"
)

Assert-FileMatches "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\UDBAMainLobbyWidgetController.cpp" @(
    'void\s+UDBAMainLobbyWidgetController::NotifyMatchFinishedClientView\(\)\s*\{(?s).*?TrackTelemetry\(TEXT\("match_finished_client_view"\).*?RefreshPlayerData\(\);.*?RefreshMatchHistory\(\);.*?\}'
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\Tests\DBAMainLobbyMatchHistoryTests.cpp" @(
    "FDBAMainLobbyMatchHistorySummaryTest",
    "MainLobby.MatchHistoryUpdatesRecentSummary",
    "UpdateMatchHistoryFromJson(Json)",
    "GetRecentMatchSummary",
    '"resultJson": "{\"winnerTeam\":\"blue\",\"schema\":\"lobby-history-test\"}"',
    '"winnerTeam": "blue"',
    '"expDelta": 1200',
    '"coin": 12',
    '"gem": 2',
    '"honor": 5',
    $recentKillsAssertionText,
    $recentDeathsAssertionText,
    $recentAssistsAssertionText,
    $recentDurationAssertionText,
    $recentCombatSummaryAssertionText,
    $recentPlayedAtAssertionText,
    $recentExpDeltaAssertionText,
    $recentCoinRewardAssertionText,
    $recentHonorRewardAssertionText,
    $recentRewardSummaryAssertionText
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Public\GameDBA\UI\Lobby\UDBAMainLobbyWidgetBase.h" @(
    "struct FDBALobbyRecentMatchSummary",
    "BackendRefreshMatchHistory",
    "HandleRecentMatchSummaryUpdated",
    "HandleRefreshMatchHistoryClicked",
    "UpdateRecentMatchSummaryText",
    "RefreshMatchHistoryButton",
    "RecentMatchResultText",
    "RecentMatchMapText",
    "RecentMatchCombatText",
    "RecentMatchPlayedAtText",
    "RecentMatchRewardText"
)

Assert-FileContains "DBA_GameClient\Source\DivineBeastsArena\Private\GameDBA\UI\Lobby\UDBAMainLobbyWidgetBase.cpp" @(
    "void UDBAMainLobbyWidgetBase::BackendRefreshMatchHistory()",
    "WidgetController->RefreshMatchHistory()",
    "void UDBAMainLobbyWidgetBase::HandleRefreshMatchHistoryClicked()",
    "BackendRefreshMatchHistory();",
    "RefreshMatchHistoryButton = Cast<UButton>(FindLobbyWidgetByNames",
    "RefreshMatchHistoryButton->OnClicked.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleRefreshMatchHistoryClicked)",
    "RefreshMatchHistoryButton->OnClicked.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleRefreshMatchHistoryClicked)",
    "HandleRecentMatchSummaryUpdated(WidgetController->GetRecentMatchSummary())",
    "void UDBAMainLobbyWidgetBase::HandleRecentMatchSummaryUpdated(const FDBALobbyRecentMatchSummary& Summary)",
    "void UDBAMainLobbyWidgetBase::UpdateRecentMatchSummaryText(const FDBALobbyRecentMatchSummary& Summary)",
    "OnRecentMatchSummaryUpdated.RemoveDynamic(this, &UDBAMainLobbyWidgetBase::HandleRecentMatchSummaryUpdated)",
    "OnRecentMatchSummaryUpdated.AddDynamic(this, &UDBAMainLobbyWidgetBase::HandleRecentMatchSummaryUpdated)",
    "RecentMatchResultText",
    "RecentMatchMapText",
    "RecentMatchCombatText",
    "RecentMatchPlayedAtText",
    "RecentMatchRewardText",
    "RecentMatchEmpty",
    "Summary.WinnerTeam",
    "Summary.Score",
    "Summary.CombatSummary",
    "Summary.PlayedAtUtc",
    "Summary.ExpDelta",
    "Summary.CoinReward",
    "Summary.HonorReward",
    "Summary.RewardSummary"
)

Write-Host "PASS: Main Lobby match history contract"
