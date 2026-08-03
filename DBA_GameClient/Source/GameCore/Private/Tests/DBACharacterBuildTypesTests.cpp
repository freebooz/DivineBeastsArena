// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 文件职责：验证 GameCore 只传递中性构筑标识符，不维护 Arena 玩法规则。
- 注意：本文件仅用于编译期契约维护；项目验收仍以人工审核为准。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GameCore/Types/DBACharacterBuildTypes.h"
#include "GameCore/Session/DBATravelTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterBuildSummaryTransportContractTest,
	"DivineBeastsArena.GameCore.Character.BuildSummary.TransportContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterBuildSummaryTransportContractTest::RunTest(const FString& Parameters)
{
	const FDBACharacterBuildSummary Summary = DBACharacterBuild::MakeBuildSummary(
		FName(TEXT("Rat")),
		FName(TEXT("Water")),
		FName(TEXT("East")),
		FName(TEXT("Rat_Water")));

	TestTrue(TEXT("完整的中性构筑标识应有效"), Summary.IsValid());
	TestEqual(TEXT("生肖标识应原样保留"), Summary.ZodiacId, FName(TEXT("Rat")));
	TestEqual(TEXT("元素标识应原样保留"), Summary.PrimaryElementId, FName(TEXT("Water")));
	TestEqual(TEXT("五营标识应原样保留"), Summary.FiveCampId, FName(TEXT("East")));
	TestEqual(TEXT("固定技能组标识应原样保留"), Summary.FixedSkillGroupId, FName(TEXT("Rat_Water")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterBuildSummaryValidationTest,
	"DivineBeastsArena.GameCore.Character.BuildSummary.ValidatesTransportDimensions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterBuildSummaryValidationTest::RunTest(const FString& Parameters)
{
	TestFalse(
		TEXT("缺少生肖标识应使构筑摘要无效"),
		DBACharacterBuild::MakeBuildSummary(NAME_None, FName(TEXT("Fire")), FName(TEXT("South")), FName(TEXT("Tiger_Fire"))).IsValid());
	TestFalse(
		TEXT("缺少元素标识应使构筑摘要无效"),
		DBACharacterBuild::MakeBuildSummary(FName(TEXT("Tiger")), NAME_None, FName(TEXT("South")), FName(TEXT("Tiger_Fire"))).IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBATravelContextBuildSummaryTransportTest,
	"DivineBeastsArena.GameCore.Character.BuildSummary.TravelContextPreservesTransportIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBATravelContextBuildSummaryTransportTest::RunTest(const FString& Parameters)
{
	FDBATravelContext Context;
	Context.SelectedZodiacId = FName(TEXT("Rat"));
	Context.SelectedElementId = FName(TEXT("Water"));
	Context.SelectedFiveCampId = FName(TEXT("East"));
	Context.FixedSkillGroupId = FName(TEXT("Rat_Water"));

	TestTrue(TEXT("旅行上下文应接受完整中性构筑身份"), Context.HasValidCharacterBuildSummary());
	const FDBACharacterBuildSummary Summary = Context.GetCharacterBuildSummary();
	TestEqual(TEXT("旅行上下文摘要应保留生肖标识"), Summary.ZodiacId, FName(TEXT("Rat")));
	TestEqual(TEXT("旅行上下文摘要应保留元素标识"), Summary.PrimaryElementId, FName(TEXT("Water")));
	TestEqual(TEXT("旅行上下文摘要应保留五营标识"), Summary.FiveCampId, FName(TEXT("East")));

	Context.FixedSkillGroupId = NAME_None;
	TestFalse(TEXT("旅行上下文应拒绝缺少固定技能组标识的摘要"), Context.HasValidCharacterBuildSummary());
	return true;
}

#endif
