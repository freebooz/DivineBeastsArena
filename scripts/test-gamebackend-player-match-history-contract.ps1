<#
验证 UE GameBackendClient 玩家战绩解析契约。

这是源码级守卫，用于让本地和 CI 证据检查在不启动 Unreal Editor 的情况下，
确认客户端稳定解析后端 `/api/players/me/matches` 结算结果字段。
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$missingFileText = -join @([char]0x7F3A, [char]0x5C11, [char]0x5FC5, [char]0x9700, [char]0x6587, [char]0x4EF6, [char]0xFF1A)
$missingContractText = -join @([char]0x7F3A, [char]0x5C11, [char]0x5951, [char]0x7EA6, [char]0x7B26, [char]0x53F7, [char]0xFF1A)
$expDeltaAssertionText = 'TestEqual(TEXT("' + (-join @([char]0x7ECF, [char]0x9A8C, [char]0x53D8, [char]0x5316, [char]0x5E94, [char]0x80FD, [char]0x89E3, [char]0x6790)) + '"), Match.ExpDelta, static_cast<int64>(900))'
$passText = (-join @([char]0x901A, [char]0x8FC7, [char]0xFF1A)) + "GameBackendClient " + (-join @([char]0x73A9, [char]0x5BB6, [char]0x6218, [char]0x7EE9, [char]0x5951, [char]0x7EA6))

function Assert-FileContains {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][string[]]$RequiredSymbols
    )

    $fullPath = Join-Path $repoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $fullPath)) {
        throw "$missingFileText$RelativePath"
    }

    $content = Get-Content -Raw -Encoding UTF8 $fullPath
    $missing = @($RequiredSymbols | Where-Object { $content -notmatch [regex]::Escape($_) })
    if ($missing.Count -gt 0) {
        throw "$RelativePath $missingContractText$($missing -join ', ')"
    }
}

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Public\GameBackendTypes.h" @(
    "FDBA_GameBackendMatchHistoryEntry",
    "FDBA_GameBackendMatchHistoryPage",
    "FString ResultJson",
    "FString WinnerTeam",
    "int64 ExpDelta",
    "TMap<FString, int64> Rewards",
    "FString PlayedAtUtc",
    "TArray<FDBA_GameBackendMatchHistoryEntry> Matches"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Public\GameBackendPlayerService.h" @(
    "TryParseMatchHistoryData",
    "FDBA_GameBackendMatchHistoryPage& OutPage",
    "FString& OutError"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\GameBackendPlayerService.cpp" @(
    "TryParseMatchHistoryData",
    'Root->TryGetObjectField(TEXT("data"), DataObj)',
    'Payload->TryGetArrayField(TEXT("matches"), Matches)',
    'Entry.ResultJson = ReadStringField(MatchObj, TEXT("resultJson"))',
    'Entry.WinnerTeam = ReadStringField(MatchObj, TEXT("winnerTeam"))',
    'Entry.ExpDelta = ReadInt64Field(MatchObj, TEXT("expDelta"))',
    "ReadNumericRewards(MatchObj, Entry.Rewards)",
    "OutPage.Matches.Add(MoveTemp(Entry))"
)

Assert-FileContains "DBA_GameClient\Plugins\GameBackendClient\Source\GameBackendClient\Private\Tests\GameBackendPlayerServiceTests.cpp" @(
    "FDBA_GameBackendPlayerMatchHistoryJsonTest",
    "MatchHistoryJsonParsesSettlementOutcome",
    "TryParseMatchHistoryData(Json, Page, Error)",
    '"resultJson": "{\"winnerTeam\":\"blue\",\"schema\":\"player-history-test\"}"',
    '"winnerTeam": "blue"',
    '"expDelta": 900',
    '"rewards"',
    $expDeltaAssertionText,
    'Match.Rewards.FindRef(TEXT("coin"))',
    'static_cast<int64>(9)',
    'Match.Rewards.FindRef(TEXT("honor"))',
    'static_cast<int64>(3)'
)

Write-Host $passText
