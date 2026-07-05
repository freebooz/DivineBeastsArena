// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient / GameBackendClient Unreal 插件自动化测试。
- 文件职责：验证玩家侧战绩 JSON 能解析为客户端 UI 可直接消费的结构化数据。
- 阅读重点：后端 `/api/players/me/matches` 的 resultJson / winnerTeam / expDelta / rewards 契约必须在 UE 客户端保持可读。
- 修改提示：保持纯字符串构造测试，不依赖真实后端、关卡或资产。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameBackendPlayerService.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBA_GameBackendPlayerMatchHistoryJsonTest,
	"DivineBeastsArena.GameBackendClient.Player.MatchHistoryJsonParsesSettlementOutcome",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBA_GameBackendPlayerMatchHistoryJsonTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
		"success": true,
		"data": {
			"matches": [
				{
					"sessionId": "session-player-history",
					"mode": "classic",
					"mapId": "arena_01",
					"team": "blue",
					"result": "win",
					"kills": 5,
					"deaths": 2,
					"assists": 7,
					"score": 1600,
					"resultJson": "{\"winnerTeam\":\"blue\",\"schema\":\"player-history-test\"}",
					"winnerTeam": "blue",
					"durationSeconds": 360,
					"expDelta": 900,
					"rewards": {
						"coin": 9,
						"honor": 3
					},
					"playedAt": "2026-07-01T01:34:39Z"
				}
			],
			"totalCount": 1,
			"page": 1,
			"pageSize": 50
		}
	})");

	FDBA_GameBackendMatchHistoryPage Page;
	FString Error;
	TestTrue(
		TEXT("战绩响应信封应能解析"),
		UDBA_GameBackendPlayerService::TryParseMatchHistoryData(Json, Page, Error));

	TestEqual(TEXT("战绩总数应能解析"), Page.TotalCount, 1);
	TestEqual(TEXT("分页页码应能解析"), Page.Page, 1);
	TestEqual(TEXT("分页大小应能解析"), Page.PageSize, 50);
	TestEqual(TEXT("应解析出一场比赛"), Page.Matches.Num(), 1);

	const FDBA_GameBackendMatchHistoryEntry& Match = Page.Matches[0];
	TestEqual(TEXT("会话标识应能解析"), Match.SessionId, FString(TEXT("session-player-history")));
	TestEqual(TEXT("模式标识应能解析"), Match.Mode, FString(TEXT("classic")));
	TestEqual(TEXT("地图标识应能解析"), Match.MapId, FString(TEXT("arena_01")));
	TestEqual(TEXT("队伍标识应能解析"), Match.Team, FString(TEXT("blue")));
	TestEqual(TEXT("比赛结果应能解析"), Match.Result, FString(TEXT("win")));
	TestEqual(TEXT("击杀数应能解析"), Match.Kills, 5);
	TestEqual(TEXT("死亡数应能解析"), Match.Deaths, 2);
	TestEqual(TEXT("助攻数应能解析"), Match.Assists, 7);
	TestEqual(TEXT("分数应能解析"), Match.Score, 1600);
	TestTrue(TEXT("结果 JSON 应保留胜利队伍字段"), Match.ResultJson.Contains(TEXT("\"winnerTeam\":\"blue\"")));
	TestEqual(TEXT("胜利队伍应能解析"), Match.WinnerTeam, FString(TEXT("blue")));
	TestEqual(TEXT("比赛时长应能解析"), Match.DurationSeconds, 360);
	TestEqual(TEXT("经验变化应能解析"), Match.ExpDelta, static_cast<int64>(900));
	TestEqual(TEXT("金币奖励应能解析"), Match.Rewards.FindRef(TEXT("coin")), static_cast<int64>(9));
	TestEqual(TEXT("荣誉奖励应能解析"), Match.Rewards.FindRef(TEXT("honor")), static_cast<int64>(3));
	TestEqual(TEXT("比赛时间应能解析"), Match.PlayedAtUtc, FString(TEXT("2026-07-01T01:34:39Z")));
	return true;
}

#endif
