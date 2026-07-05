// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端自动化测试。
- 文件职责：验证主大厅 Controller 能把后端战绩 JSON 转为 UI 可消费的最近战绩摘要。
- 阅读重点：Lobby UI 不应手写解析 `/api/players/me/matches`，应消费 Controller 缓存的结构化摘要。
- 修改提示：保持纯 Controller/JSON 测试，不依赖真实后端、关卡或资产。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetController.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAMainLobbyMatchHistorySummaryTest,
	"DivineBeastsArena.UI.MainLobby.MatchHistoryUpdatesRecentSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAMainLobbyMatchHistorySummaryTest::RunTest(const FString& Parameters)
{
	UDBAMainLobbyWidgetController* Controller = NewObject<UDBAMainLobbyWidgetController>();
	TestNotNull(TEXT("主大厅控制器应能创建"), Controller);

	const FString Json = TEXT(R"({
		"success": true,
		"data": {
			"matches": [
				{
					"sessionId": "session-ui-history",
					"mode": "classic",
					"mapId": "arena_01",
					"team": "blue",
					"result": "win",
					"kills": 8,
					"deaths": 1,
					"assists": 6,
					"score": 2200,
					"resultJson": "{\"winnerTeam\":\"blue\",\"schema\":\"lobby-history-test\"}",
					"winnerTeam": "blue",
					"durationSeconds": 420,
					"expDelta": 1200,
					"rewards": {
						"coin": 12,
						"gem": 2,
						"honor": 5
					},
					"playedAt": "2026-07-01T02:00:00Z"
				}
			],
			"totalCount": 1,
			"page": 1,
			"pageSize": 50
		}
	})");

	TestTrue(TEXT("战绩 JSON 应更新最近战绩摘要"), Controller->UpdateMatchHistoryFromJson(Json));

	const FDBALobbyRecentMatchSummary& Summary = Controller->GetRecentMatchSummary();
	TestTrue(TEXT("最近战绩摘要应有效"), Summary.bHasMatch);
	TestEqual(TEXT("最近会话标识应被解析"), Summary.SessionId, FString(TEXT("session-ui-history")));
	TestEqual(TEXT("最近比赛结果应被解析"), Summary.Result, FString(TEXT("win")));
	TestEqual(TEXT("最近胜利队伍应被解析"), Summary.WinnerTeam, FString(TEXT("blue")));
	TestEqual(TEXT("最近地图标识应被解析"), Summary.MapId, FString(TEXT("arena_01")));
	TestEqual(TEXT("最近分数应被解析"), Summary.Score, 2200);
	TestEqual(TEXT("最近击杀数应被解析"), Summary.Kills, 8);
	TestEqual(TEXT("最近死亡数应被解析"), Summary.Deaths, 1);
	TestEqual(TEXT("最近助攻数应被解析"), Summary.Assists, 6);
	TestEqual(TEXT("最近持续时间应被解析"), Summary.DurationSeconds, 420);
	TestEqual(TEXT("最近战斗摘要应被格式化"), Summary.CombatSummary, FString(TEXT("KDA 8/1/6 / 07:00")));
	TestEqual(TEXT("最近游玩时间应被解析"), Summary.PlayedAtUtc, FString(TEXT("2026-07-01T02:00:00Z")));
	TestEqual(TEXT("最近经验变化应被解析"), Summary.ExpDelta, static_cast<int64>(1200));
	TestEqual(TEXT("最近金币奖励应被解析"), Summary.CoinReward, static_cast<int64>(12));
	TestEqual(TEXT("最近荣誉奖励应被解析"), Summary.HonorReward, static_cast<int64>(5));
	TestEqual(TEXT("最近奖励摘要应包含全部数值奖励"), Summary.RewardSummary, FString(TEXT("coin +12 / gem +2 / honor +5")));
	return true;
}

#endif
