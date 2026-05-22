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
#include "GameCore/Account/DBAOnlineAccountJson.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountJsonLoginTest,
	"DivineBeastsArena.GameCore.Account.OnlineJson.ParseLoginResponse",
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
	TestTrue(TEXT("Login response should parse"), FDBAOnlineAccountJson::ParseLoginResponse(Json, Response, Error));
	TestTrue(TEXT("Login should succeed"), Response.bSuccess);
	TestEqual(TEXT("Token should parse"), Response.SessionToken, FString(TEXT("session-token")));
	TestEqual(TEXT("AccountId should parse"), Response.AccountInfo.AccountId.ToString(), FString(TEXT("account_001")));
	TestEqual(TEXT("DisplayName should parse"), Response.AccountInfo.DisplayName, FString(TEXT("Player001")));
	TestEqual(TEXT("LoginType should parse"), Response.AccountInfo.LoginType, EDBALoginType::Email);
	TestEqual(TEXT("Status should parse"), Response.AccountInfo.Status, EDBAAccountStatus::Normal);
	TestEqual(TEXT("Level should parse"), Response.AccountInfo.Level, 7);
	TestEqual(TEXT("Experience should parse"), Response.AccountInfo.Experience, 120);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountJsonCharactersTest,
	"DivineBeastsArena.GameCore.Account.OnlineJson.ParseCharacters",
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
	TestTrue(TEXT("Characters should parse"), FDBAOnlineAccountJson::ParseCharacterListResponse(Json, Characters, Error));
	TestEqual(TEXT("Character count"), Characters.Num(), 1);
	if (Characters.Num() == 1)
	{
		TestEqual(TEXT("CharacterId"), Characters[0].CharacterId.ToString(), FString(TEXT("char_001")));
		TestEqual(TEXT("CharacterName"), Characters[0].CharacterName, FString(TEXT("WaterRat")));
		TestEqual(TEXT("Zodiac"), Characters[0].Zodiac, EDBAZodiac::Rat);
		TestEqual(TEXT("PrimaryElement"), Characters[0].PrimaryElement, EDBAElement::Water);
		TestEqual(TEXT("FiveCamp"), Characters[0].FiveCamp, EDBAFiveCamp::West);
		TestEqual(TEXT("FixedSkillGroupId"), Characters[0].FixedSkillGroupId, FName(TEXT("Rat_Water")));
		TestEqual(TEXT("MaxHealth"), Characters[0].CoreAttributes.MaxHealth, 2070.0f);
		TestEqual(TEXT("Defense"), Characters[0].CoreAttributes.Defense, 44.0f);
		TestEqual(TEXT("EnergyRegen"), Characters[0].CoreAttributes.EnergyRegen, 12.0f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountJsonWrappedCharactersTest,
	"DivineBeastsArena.GameCore.Account.OnlineJson.ParseWrappedCharacters",
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
	TestTrue(TEXT("Wrapped characters should parse"), FDBAOnlineAccountJson::ParseCharacterListResponse(Json, Characters, Error));
	TestEqual(TEXT("Wrapped character count"), Characters.Num(), 1);
	if (Characters.Num() == 1)
	{
		TestEqual(TEXT("Wrapped character id"), Characters[0].CharacterId.ToString(), FString(TEXT("char_002")));
		TestEqual(TEXT("Wrapped character zodiac"), Characters[0].Zodiac, EDBAZodiac::Tiger);
		TestEqual(TEXT("Wrapped character element"), Characters[0].PrimaryElement, EDBAElement::Fire);
	}
	return true;
}

#endif
