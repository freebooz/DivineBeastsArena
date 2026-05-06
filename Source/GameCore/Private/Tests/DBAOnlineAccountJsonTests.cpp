// Copyright FreeboozStudio. All Rights Reserved.

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

#endif
