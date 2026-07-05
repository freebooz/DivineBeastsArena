// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Framework/DBAUrlOptions.h"
#include "GameCore/Character/DBACharacterBuildTypes.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAUrlOptionsDecodeTest,
	"DivineBeastsArena.GameDBA.Framework.UrlOptions.DecodesEscapedValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAUrlOptionsDecodeTest::RunTest(const FString& Parameters)
{
	const FString Options = TEXT("?PlayerId=player-1&PlayerSessionToken=iz%2FhZlJiO%2BeVpWJoH2r8dCGwEgg%2Fvcn%2BjTJLPI%2FJgQQ%3D&DBAZodiac=Rat&DBAElement=Water&DBAFixedSkillGroupId=Rat_Water");

	TestEqual(
		TEXT("转义后的玩家会话令牌应完成 URL 解码"),
		DBAUrlOptions::ExtractUrlOption(Options, TEXT("PlayerSessionToken")),
		FString(TEXT("iz/hZlJiO+eVpWJoH2r8dCGwEgg/vcn+jTJLPI/JgQQ=")));
	TestEqual(
		TEXT("未转义的构筑字段应保持原值"),
		DBAUrlOptions::ExtractUrlOption(Options, TEXT("DBAFixedSkillGroupId")),
		FString(TEXT("Rat_Water")));
	TestEqual(
		TEXT("缺失参数应返回空字符串"),
		DBAUrlOptions::ExtractUrlOption(Options, TEXT("Missing")),
		FString());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAUrlOptionsBuildSummaryAdmissionTest,
	"DivineBeastsArena.GameDBA.Framework.UrlOptions.ValidatesDedicatedServerBuildSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAUrlOptionsBuildSummaryAdmissionTest::RunTest(const FString& Parameters)
{
	const FString ValidOptions = TEXT("?PlayerId=player-1&PlayerSessionToken=token-1&DBAZodiac=Rat&DBAElement=Water&DBAFiveCamp=East&DBAFixedSkillGroupId=Rat_Water&DBATeamId=1");

	FDBACharacterBuildSummary Summary;
	TestTrue(
		TEXT("专用服务器参数应接受匹配的冻结构筑摘要"),
		DBAUrlOptions::TryExtractCharacterBuildSummary(ValidOptions, Summary));
	TestEqual(TEXT("生肖应从旅行参数解析"), Summary.Zodiac, EDBAZodiac::Rat);
	TestEqual(TEXT("元素应从旅行参数解析"), Summary.PrimaryElement, EDBAElement::Water);
	TestEqual(TEXT("五营应作为仅表现身份保留"), Summary.FiveCamp, EDBAFiveCamp::East);
	TestEqual(TEXT("固定技能组标识应从旅行参数保留"), Summary.FixedSkillGroupId, FName(TEXT("Rat_Water")));

	int32 TeamId = 0;
	TestTrue(
		TEXT("专用服务器参数应接受队伍标识"),
		DBAUrlOptions::TryExtractTeamId(ValidOptions, TeamId));
	TestEqual(TEXT("队伍标识应解析为正数权威队伍编号"), TeamId, 1);

	int32 AliasTeamId = 0;
	TestTrue(
		TEXT("专用服务器参数应接受后端旅行地址中的队伍标识别名"),
		DBAUrlOptions::TryExtractTeamId(TEXT("?TeamId=2"), AliasTeamId));
	TestEqual(TEXT("队伍标识别名应解析为正数权威队伍编号"), AliasTeamId, 2);

	FDBACharacterBuildSummary MixedCaseSummary;
	TestTrue(
		TEXT("专用服务器参数应接受大小写混合的稳定名称"),
		DBAUrlOptions::TryExtractCharacterBuildSummary(
			TEXT("?DBAZodiac=rat&DBAElement=WATER&DBAFiveCamp=east&DBAFixedSkillGroupId=rat_water"),
			MixedCaseSummary));
	TestEqual(
		TEXT("接受的大小写混合固定技能组标识应为后端上报标准化"),
		MixedCaseSummary.FixedSkillGroupId.ToString(),
		FString(TEXT("Rat_Water")));

	FDBACharacterBuildSummary TamperedSummary;
	TestFalse(
		TEXT("专用服务器参数应拒绝被篡改的固定技能组标识"),
		DBAUrlOptions::TryExtractCharacterBuildSummary(
			TEXT("?DBAZodiac=Rat&DBAElement=Water&DBAFiveCamp=North&DBAFixedSkillGroupId=Rat_Fire"),
			TamperedSummary));

	FDBACharacterBuildSummary MissingSummary;
	TestFalse(
		TEXT("专用服务器参数应拒绝缺少冻结身份字段"),
		DBAUrlOptions::TryExtractCharacterBuildSummary(
			TEXT("?DBAZodiac=Rat&DBAFiveCamp=East&DBAFixedSkillGroupId=Rat_Water"),
			MissingSummary));

	int32 MissingTeamId = 99;
	TestFalse(
		TEXT("专用服务器参数应拒绝缺少队伍标识"),
		DBAUrlOptions::TryExtractTeamId(TEXT("?DBAZodiac=Rat&DBAElement=Water&DBAFixedSkillGroupId=Rat_Water"), MissingTeamId));
	TestEqual(TEXT("拒绝缺少队伍标识时应清空输出队伍编号"), MissingTeamId, 0);

	int32 NonPositiveTeamId = 99;
	TestFalse(
		TEXT("专用服务器参数应拒绝非正数队伍标识"),
		DBAUrlOptions::TryExtractTeamId(TEXT("?DBATeamId=0"), NonPositiveTeamId));
	TestEqual(TEXT("拒绝非正数队伍标识时应清空输出队伍编号"), NonPositiveTeamId, 0);

	return true;
}

#endif
