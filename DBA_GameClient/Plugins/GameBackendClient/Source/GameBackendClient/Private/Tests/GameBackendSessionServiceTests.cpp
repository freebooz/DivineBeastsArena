// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient / GameBackendClient Unreal 插件自动化测试。
- 文件职责：验证 Dedicated Server travel URL 能携带后端冻结角色构筑摘要。
- 阅读重点：DBAZodiac / DBAElement / DBAFiveCamp / DBAFixedSkillGroupId 必须来自后端连接摘要并进入 URL options。
- 修改提示：保持纯字符串构造测试，不依赖真实后端、关卡或资产。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameBackendSessionService.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBA_GameBackendSessionTravelUrlBuildSummaryTest,
	"DivineBeastsArena.GameBackendClient.Session.BuildTravelUrlIncludesFrozenBuildSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBA_GameBackendSessionTravelUrlBuildSummaryTest::RunTest(const FString& Parameters)
{
	const FString Url = UDBA_GameBackendSessionService::BuildTravelUrl(
		TEXT("127.0.0.1"),
		7777,
		TEXT("session-001"),
		TEXT("token/with+specials"),
		1,
		TEXT("Rat"),
		TEXT("Water"),
		TEXT("East"),
		TEXT("Rat_Water"));

	TestTrue(TEXT("旅行地址应包含会话标识"), Url.Contains(TEXT("SessionId=session-001")));
	TestTrue(TEXT("旅行地址应对玩家会话令牌进行 URL 编码"), Url.Contains(TEXT("PlayerSessionToken=token%2Fwith%2Bspecials")));
	TestTrue(TEXT("旅行地址应包含队伍标识"), Url.Contains(TEXT("DBATeamId=1")));
	TestTrue(TEXT("旅行地址应包含生肖标识"), Url.Contains(TEXT("DBAZodiac=Rat")));
	TestTrue(TEXT("旅行地址应包含元素标识"), Url.Contains(TEXT("DBAElement=Water")));
	TestTrue(TEXT("旅行地址应包含五营标识"), Url.Contains(TEXT("DBAFiveCamp=East")));
	TestTrue(TEXT("旅行地址应包含固定技能组标识"), Url.Contains(TEXT("DBAFixedSkillGroupId=Rat_Water")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBA_GameBackendSessionConnectionJsonBuildSummaryTest,
	"DivineBeastsArena.GameBackendClient.Session.ConnectionJsonBuildsTravelUrlWithNestedBuildSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBA_GameBackendSessionConnectionJsonBuildSummaryTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
		"connection": {
			"ip": "127.0.0.1",
			"port": 7777,
			"sessionId": "legacy-session",
			"playerSessionToken": "nested-token",
			"teamId": 2,
			"characterBuildSummary": {
				"zodiac": "Tiger",
				"primaryElement": "Fire",
				"fiveCamp": "South",
				"fixedSkillGroupId": "Tiger_Fire"
			}
		}
	})");

	FString Url;
	TestTrue(
		TEXT("嵌套连接 JSON 应能构建旅行地址"),
		UDBA_GameBackendSessionService::TryBuildTravelUrlFromConnectionData(Json, TEXT("override-session"), Url));

	TestTrue(TEXT("覆盖会话标识应优先使用"), Url.Contains(TEXT("SessionId=override-session")));
	TestTrue(TEXT("嵌套玩家令牌应被写入"), Url.Contains(TEXT("PlayerSessionToken=nested-token")));
	TestTrue(TEXT("嵌套队伍标识应被写入"), Url.Contains(TEXT("DBATeamId=2")));
	TestTrue(TEXT("嵌套生肖标识应被写入"), Url.Contains(TEXT("DBAZodiac=Tiger")));
	TestTrue(TEXT("嵌套元素标识应被写入"), Url.Contains(TEXT("DBAElement=Fire")));
	TestTrue(TEXT("嵌套五营标识应被写入"), Url.Contains(TEXT("DBAFiveCamp=South")));
	TestTrue(TEXT("嵌套固定技能组标识应被写入"), Url.Contains(TEXT("DBAFixedSkillGroupId=Tiger_Fire")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBA_GameBackendSessionConnectionAliasJsonTest,
	"DivineBeastsArena.GameBackendClient.Session.ConnectionJsonAcceptsNestedServerAliases",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBA_GameBackendSessionConnectionAliasJsonTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
		"connection": {
			"serverIp": "10.0.0.42",
			"serverPort": 7788,
			"sessionId": "alias-session",
			"sessionToken": "alias-token",
			"teamId": 1,
			"characterBuildSummary": {
				"zodiac": "Dragon",
				"primaryElement": "Wood",
				"fiveCamp": "East",
				"fixedSkillGroupId": "Dragon_Wood"
			}
		}
	})");

	FString Url;
	TestTrue(
		TEXT("嵌套连接别名应能构建旅行地址"),
		UDBA_GameBackendSessionService::TryBuildTravelUrlFromConnectionData(Json, FString(), Url));

	TestTrue(TEXT("嵌套服务器地址应被使用"), Url.StartsWith(TEXT("10.0.0.42:7788")));
	TestTrue(TEXT("嵌套会话标识应被写入"), Url.Contains(TEXT("SessionId=alias-session")));
	TestTrue(TEXT("嵌套会话令牌别名应被写入"), Url.Contains(TEXT("PlayerSessionToken=alias-token")));
	TestTrue(TEXT("嵌套队伍标识别名应被写入"), Url.Contains(TEXT("DBATeamId=1")));
	TestTrue(TEXT("嵌套生肖标识别名应被写入"), Url.Contains(TEXT("DBAZodiac=Dragon")));
	TestTrue(TEXT("嵌套元素标识别名应被写入"), Url.Contains(TEXT("DBAElement=Wood")));
	TestTrue(TEXT("嵌套固定技能组标识别名应被写入"), Url.Contains(TEXT("DBAFixedSkillGroupId=Dragon_Wood")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBA_GameBackendSessionEnvelopeJsonTest,
	"DivineBeastsArena.GameBackendClient.Session.ConnectionJsonAcceptsResponseEnvelopeData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBA_GameBackendSessionEnvelopeJsonTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
		"success": true,
		"data": {
			"serverIp": "172.16.0.9",
			"serverPort": 7799,
			"sessionId": "envelope-session",
			"sessionToken": "envelope-token",
			"teamId": 2,
			"characterBuildSummary": {
				"zodiac": "Snake",
				"primaryElement": "Gold",
				"fiveCamp": "West",
				"fixedSkillGroupId": "Snake_Gold"
			}
		}
	})");

	FString Url;
	TestTrue(
		TEXT("响应信封数据应能构建旅行地址"),
		UDBA_GameBackendSessionService::TryBuildTravelUrlFromConnectionData(Json, FString(), Url));

	TestTrue(TEXT("信封服务器地址应被使用"), Url.StartsWith(TEXT("172.16.0.9:7799")));
	TestTrue(TEXT("信封会话标识应被写入"), Url.Contains(TEXT("SessionId=envelope-session")));
	TestTrue(TEXT("信封会话令牌别名应被写入"), Url.Contains(TEXT("PlayerSessionToken=envelope-token")));
	TestTrue(TEXT("信封队伍标识应被写入"), Url.Contains(TEXT("DBATeamId=2")));
	TestTrue(TEXT("信封生肖标识应被写入"), Url.Contains(TEXT("DBAZodiac=Snake")));
	TestTrue(TEXT("信封元素标识应被写入"), Url.Contains(TEXT("DBAElement=Gold")));
	TestTrue(TEXT("信封固定技能组标识应被写入"), Url.Contains(TEXT("DBAFixedSkillGroupId=Snake_Gold")));
	return true;
}

#endif
