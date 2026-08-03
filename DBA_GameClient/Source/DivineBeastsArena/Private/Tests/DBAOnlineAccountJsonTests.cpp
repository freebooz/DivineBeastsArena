// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Dom/JsonObject.h"
#include "GameDBA/Frontend/Account/DBAOnlineAccountJson.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountJsonLoginTest,
	"DivineBeastsArena.Frontend.Account.OnlineJson.ParseLoginResponse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAOnlineAccountJsonLoginTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
		"success": true,
		"token": "session-token",
		"account": {
			"accountId": "account_001",
			"displayName": "Player001",
			"loginType": "Email",
			"status": "Normal",
			"level": 7,
			"experience": 120
		}
	})");

	FDBALoginResponse Response;
	FString Error;
	TestTrue(TEXT("登录响应应能解析"), FDBAOnlineAccountJson::ParseLoginResponse(Json, Response, Error));
	TestTrue(TEXT("登录应成功"), Response.bSuccess);
	TestEqual(TEXT("会话令牌应能解析"), Response.SessionToken, FString(TEXT("session-token")));
	TestEqual(TEXT("账号标识应能解析"), Response.AccountInfo.AccountId.ToString(), FString(TEXT("account_001")));
	TestEqual(TEXT("显示名称应能解析"), Response.AccountInfo.DisplayName, FString(TEXT("Player001")));
	TestEqual(TEXT("登录类型应能解析"), Response.AccountInfo.LoginType, EDBALoginType::Email);
	TestEqual(TEXT("账号状态应能解析"), Response.AccountInfo.Status, EDBAAccountStatus::Normal);
	TestEqual(TEXT("等级应能解析"), Response.AccountInfo.Level, 7);
	TestEqual(TEXT("经验应能解析"), Response.AccountInfo.Experience, 120);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountJsonGuestRequestTest,
	"DivineBeastsArena.Frontend.Account.OnlineJson.BuildGuestLoginRequest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAOnlineAccountJsonGuestRequestTest::RunTest(const FString& Parameters)
{
	const FString Json = FDBAOnlineAccountJson::BuildGuestLoginRequest(TEXT("device_001"), TEXT("UnrealClient"), TEXT("Windows"));

	TSharedPtr<FJsonObject> Object;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	TestTrue(TEXT("游客请求 JSON 应能解析"), FJsonSerializer::Deserialize(Reader, Object));
	TestTrue(TEXT("游客请求对象应有效"), Object.IsValid());
	if (Object.IsValid())
	{
		TestEqual(TEXT("游客请求应包含 deviceId"), Object->GetStringField(TEXT("deviceId")), FString(TEXT("device_001")));
		TestEqual(TEXT("游客请求应包含 deviceName"), Object->GetStringField(TEXT("deviceName")), FString(TEXT("UnrealClient")));
		TestEqual(TEXT("游客请求应包含 platform"), Object->GetStringField(TEXT("platform")), FString(TEXT("Windows")));
		TestFalse(TEXT("游客请求不应发送账号登录类型"), Object->HasField(TEXT("loginType")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountJsonWrappedTokenDisplayNameTest,
	"DivineBeastsArena.Frontend.Account.OnlineJson.ParseWrappedTokenDisplayName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAOnlineAccountJsonWrappedTokenDisplayNameTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
		"success": true,
		"data": {
			"accessToken": "guest-access-token",
			"refreshToken": "guest-refresh-token",
			"playerId": "guest_001",
			"displayName": "AzureGuest",
			"expiresIn": 3600
		}
	})");

	FDBALoginResponse Response;
	FString Error;
	TestTrue(TEXT("游客登录响应应能解析"), FDBAOnlineAccountJson::ParseLoginResponse(Json, Response, Error));
	TestTrue(TEXT("游客登录应成功"), Response.bSuccess);
	TestEqual(TEXT("游客访问令牌应能解析"), Response.SessionToken, FString(TEXT("guest-access-token")));
	TestEqual(TEXT("游客刷新令牌应能解析"), Response.RefreshToken, FString(TEXT("guest-refresh-token")));
	TestEqual(TEXT("游客玩家标识应能解析"), Response.PlayerId, FString(TEXT("guest_001")));
	TestEqual(TEXT("游客显示名称应使用后端 displayName"), Response.AccountInfo.DisplayName, FString(TEXT("AzureGuest")));
	TestEqual(TEXT("游客账号标识应能解析"), Response.AccountInfo.AccountId.ToString(), FString(TEXT("guest_001")));
	TestEqual(TEXT("包裹令牌响应不应推断端点专属登录类型"), Response.AccountInfo.LoginType, EDBALoginType::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountJsonRefreshTypeTest,
	"DivineBeastsArena.Frontend.Account.OnlineJson.ParseRefreshDoesNotAssumeGuest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAOnlineAccountJsonRefreshTypeTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
		"success": true,
		"data": {
			"accessToken": "refreshed-access-token",
			"refreshToken": "refreshed-refresh-token",
			"playerId": "email_player_001",
			"nickname": "EmailPlayer"
		}
	})");

	FDBALoginResponse Response;
	FString Error;
	TestTrue(TEXT("刷新令牌响应应能解析"), FDBAOnlineAccountJson::ParseLoginResponse(Json, Response, Error));
	TestTrue(TEXT("刷新令牌响应应成功"), Response.bSuccess);
	TestEqual(TEXT("刷新令牌响应应解析玩家标识"), Response.PlayerId, FString(TEXT("email_player_001")));
	TestEqual(TEXT("刷新令牌响应应解析显示名称"), Response.AccountInfo.DisplayName, FString(TEXT("EmailPlayer")));
	TestEqual(TEXT("刷新令牌响应不应仅凭 JSON 形状归类为游客"), Response.AccountInfo.LoginType, EDBALoginType::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountJsonCharactersTest,
	"DivineBeastsArena.Frontend.Account.OnlineJson.ParseCharacters",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAOnlineAccountJsonCharactersTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
		"success": true,
		"characters": [
			{
				"characterId": "char_001",
				"characterName": "WaterRat",
				"zodiac": "Rat",
				"primaryElement": "Water",
				"fiveCamp": "West",
				"fixedSkillGroupId": "Rat_Water",
				"level": 3,
				"coreAttributes": {
					"maxHealth": 2070,
					"attackPower": 100,
					"defense": 44,
					"moveSpeed": 380,
					"maxEnergy": 100,
					"energyRegen": 12,
					"criticalRate": 5,
					"criticalMultiplier": 200
				}
			}
		]
	})");

	TArray<FDBACharacterSummary> Characters;
	FString Error;
	TestTrue(TEXT("角色列表应能解析"), FDBAOnlineAccountJson::ParseCharacterListResponse(Json, Characters, Error));
	TestEqual(TEXT("角色数量应匹配"), Characters.Num(), 1);
	if (Characters.Num() == 1)
	{
		TestEqual(TEXT("角色标识应能解析"), Characters[0].CharacterId.ToString(), FString(TEXT("char_001")));
		TestEqual(TEXT("角色名称应能解析"), Characters[0].CharacterName, FString(TEXT("WaterRat")));
		TestEqual(TEXT("生肖应能解析"), Characters[0].Zodiac, EDBAZodiac::Rat);
		TestEqual(TEXT("主元素应能解析"), Characters[0].PrimaryElement, EDBAElement::Water);
		TestEqual(TEXT("五营应能解析"), Characters[0].FiveCamp, EDBAFiveCamp::West);
		TestEqual(TEXT("固定技能组标识应能解析"), Characters[0].FixedSkillGroupId, FName(TEXT("Rat_Water")));
		TestEqual(TEXT("最大生命应能解析"), Characters[0].CoreAttributes.MaxHealth, 2070.0f);
		TestEqual(TEXT("防御应能解析"), Characters[0].CoreAttributes.Defense, 44.0f);
		TestEqual(TEXT("能量回复应能解析"), Characters[0].CoreAttributes.EnergyRegen, 12.0f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountJsonWrappedCharactersTest,
	"DivineBeastsArena.Frontend.Account.OnlineJson.ParseWrappedCharacters",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAOnlineAccountJsonWrappedCharactersTest::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
		"success": true,
		"data": {
			"selectedCharacterId": "char_002",
			"characters": [
				{
					"characterId": "char_002",
					"characterName": "FireTiger",
					"zodiac": "Tiger",
					"primaryElement": "Fire",
					"fiveCamp": "South",
					"fixedSkillGroupId": "Tiger_Fire",
					"level": 5
				}
			]
		}
	})");

	TArray<FDBACharacterSummary> Characters;
	FString Error;
	TestTrue(TEXT("包裹角色列表应能解析"), FDBAOnlineAccountJson::ParseCharacterListResponse(Json, Characters, Error));
	TestEqual(TEXT("包裹角色数量应匹配"), Characters.Num(), 1);
	if (Characters.Num() == 1)
	{
		TestEqual(TEXT("包裹角色标识应能解析"), Characters[0].CharacterId.ToString(), FString(TEXT("char_002")));
		TestEqual(TEXT("包裹角色生肖应能解析"), Characters[0].Zodiac, EDBAZodiac::Tiger);
		TestEqual(TEXT("包裹角色元素应能解析"), Characters[0].PrimaryElement, EDBAElement::Fire);
	}
	return true;
}

#endif
