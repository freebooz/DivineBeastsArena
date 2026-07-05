// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient / GameBackendClient Unreal 插件自动化测试。
- 文件职责：验证 Dedicated Server Runtime API payload 构造，不依赖真实后端、关卡或资产。
- 阅读重点：match results payload 必须与后端 RuntimeMatchResultsRequest 字段保持一致。
- 修改提示：新增 Runtime 上报字段时，请同步本文件和对应后端契约脚本。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameBackendRuntimeService.h"
#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBA_GameBackendRuntimeMatchResultsPayloadTest,
	"DivineBeastsArena.GameBackendClient.Runtime.BuildMatchResultsPayload",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBA_GameBackendRuntimeMatchResultsPayloadTest::RunTest(const FString& Parameters)
{
	FDBA_GameBackendRuntimePlayerResult Player;
	Player.PlayerId = TEXT("player-001");
	Player.Team = TEXT("blue");
	Player.Result = TEXT("win");
	Player.Kills = 7;
	Player.Deaths = 1;
	Player.Assists = 4;
	Player.Score = 1234;
	Player.ExpDelta = 1500;
	Player.Rewards.Add(TEXT("coin"), 25);
	Player.Rewards.Add(TEXT("honor"), 3);

	const FString Payload = UDBA_GameBackendRuntimeService::BuildMatchResultsPayload(
		TEXT("server-001"),
		TEXT("session-001"),
		TEXT("runtime-token-001"),
		TEXT("match-result-001"),
		TEXT("{\"winner\":\"blue\"}"),
		{ Player });

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Payload);
	TestTrue(TEXT("载荷应能解析为 JSON"), FJsonSerializer::Deserialize(Reader, Root));
	TestTrue(TEXT("载荷根对象应有效"), Root.IsValid());
	if (!Root.IsValid())
	{
		return false;
	}

	TestEqual(TEXT("服务器标识应被序列化"), Root->GetStringField(TEXT("serverId")), FString(TEXT("server-001")));
	TestEqual(TEXT("会话标识应被序列化"), Root->GetStringField(TEXT("sessionId")), FString(TEXT("session-001")));
	TestEqual(TEXT("运行时令牌应被序列化"), Root->GetStringField(TEXT("runtimeToken")), FString(TEXT("runtime-token-001")));
	TestEqual(TEXT("幂等键应被序列化"), Root->GetStringField(TEXT("idempotencyKey")), FString(TEXT("match-result-001")));
	TestEqual(TEXT("结果 JSON 应被序列化"), Root->GetStringField(TEXT("resultJson")), FString(TEXT("{\"winner\":\"blue\"}")));

	const TArray<TSharedPtr<FJsonValue>>* Players = nullptr;
	TestTrue(TEXT("玩家数组应存在"), Root->TryGetArrayField(TEXT("players"), Players));
	TestTrue(TEXT("玩家数组应包含一名玩家"), Players && Players->Num() == 1);
	if (!Players || Players->Num() != 1)
	{
		return false;
	}

	const TSharedPtr<FJsonObject> PlayerJson = (*Players)[0]->AsObject();
	TestTrue(TEXT("玩家条目应为对象"), PlayerJson.IsValid());
	if (!PlayerJson.IsValid())
	{
		return false;
	}

	TestEqual(TEXT("玩家标识应被序列化"), PlayerJson->GetStringField(TEXT("playerId")), FString(TEXT("player-001")));
	TestEqual(TEXT("队伍应被序列化"), PlayerJson->GetStringField(TEXT("team")), FString(TEXT("blue")));
	TestEqual(TEXT("比赛结果应被序列化"), PlayerJson->GetStringField(TEXT("result")), FString(TEXT("win")));
	TestEqual(TEXT("击杀数应被序列化"), static_cast<int32>(PlayerJson->GetNumberField(TEXT("kills"))), 7);
	TestEqual(TEXT("死亡数应被序列化"), static_cast<int32>(PlayerJson->GetNumberField(TEXT("deaths"))), 1);
	TestEqual(TEXT("助攻数应被序列化"), static_cast<int32>(PlayerJson->GetNumberField(TEXT("assists"))), 4);
	TestEqual(TEXT("得分应被序列化"), static_cast<int32>(PlayerJson->GetNumberField(TEXT("score"))), 1234);
	TestEqual(TEXT("经验变化应被序列化"), static_cast<int64>(PlayerJson->GetNumberField(TEXT("expDelta"))), static_cast<int64>(1500));

	const TSharedPtr<FJsonObject> RewardsJson = PlayerJson->GetObjectField(TEXT("rewards"));
	TestTrue(TEXT("奖励应被序列化为对象"), RewardsJson.IsValid());
	if (RewardsJson.IsValid())
	{
		TestEqual(TEXT("金币奖励应被序列化"), static_cast<int32>(RewardsJson->GetNumberField(TEXT("coin"))), 25);
		TestEqual(TEXT("荣誉奖励应被序列化"), static_cast<int32>(RewardsJson->GetNumberField(TEXT("honor"))), 3);
	}

	return true;
}

#endif
