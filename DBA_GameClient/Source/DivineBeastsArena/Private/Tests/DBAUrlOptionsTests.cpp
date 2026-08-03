// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Framework/Travel/DBAUrlOptions.h"
#include "GameCore/Types/DBACharacterBuildTypes.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAUrlOptionsDecodeTest,
	"DivineBeastsArena.GameDBA.Framework.UrlOptions.DecodesEscapedValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAUrlOptionsDecodeTest::RunTest(const FString& Parameters)
{
	const FString Options = TEXT("?PlayerId=player-1&PlayerSessionToken=iz%2FhZlJiO%2BeVpWJoH2r8dCGwEgg%2Fvcn%2BjTJLPI%2FJgQQ%3D&DBAZodiac=Rat&DBAElement=Water&DBAFixedSkillGroupId=Rat_Water");
	TestEqual(TEXT("转义后的玩家会话令牌应完成 URL 解码"), DBAUrlOptions::ExtractUrlOption(Options, TEXT("PlayerSessionToken")), FString(TEXT("iz/hZlJiO+eVpWJoH2r8dCGwEgg/vcn+jTJLPI/JgQQ=")));
	TestEqual(TEXT("缺失参数应返回空字符串"), DBAUrlOptions::ExtractUrlOption(Options, TEXT("Missing")), FString());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAUrlOptionsBuildSummaryTransportTest,
	"DivineBeastsArena.GameDBA.Framework.UrlOptions.ParsesBuildTransportIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAUrlOptionsBuildSummaryTransportTest::RunTest(const FString& Parameters)
{
	FDBACharacterBuildSummary Summary;
	TestTrue(TEXT("专用服务器参数应接受完整中性构筑身份"), DBAUrlOptions::TryExtractCharacterBuildSummary(TEXT("?DBAZodiac=Rat&DBAElement=Water&DBAFiveCamp=East&DBAFixedSkillGroupId=Rat_Water"), Summary));
	TestEqual(TEXT("生肖标识应从旅行参数保留"), Summary.ZodiacId, FName(TEXT("Rat")));
	TestEqual(TEXT("元素标识应从旅行参数保留"), Summary.PrimaryElementId, FName(TEXT("Water")));
	TestEqual(TEXT("五营标识应从旅行参数保留"), Summary.FiveCampId, FName(TEXT("East")));
	TestEqual(TEXT("固定技能组标识应从旅行参数保留"), Summary.FixedSkillGroupId, FName(TEXT("Rat_Water")));

	FDBACharacterBuildSummary MissingSummary;
	TestFalse(TEXT("缺少字段的旅行参数应被拒绝"), DBAUrlOptions::TryExtractCharacterBuildSummary(TEXT("?DBAZodiac=Rat&DBAFiveCamp=East&DBAFixedSkillGroupId=Rat_Water"), MissingSummary));
	return true;
}

#endif
