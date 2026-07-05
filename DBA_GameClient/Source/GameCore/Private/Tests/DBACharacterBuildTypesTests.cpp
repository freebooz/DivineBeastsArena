// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：GameCore 自动化测试，验证角色创建构建摘要的基础契约。
- 阅读重点：Zodiac / Element / FiveCamp 的解耦规则，以及 FixedSkillGroupId 的稳定生成。
- 修改提示：测试应继续保持纯 GameCore 依赖，不引用 DivineBeastsArena 项目层类型。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GameCore/Character/DBACharacterBuildTypes.h"
#include "GameCore/Session/DBATravelTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterBuildSummaryFixedSkillGroupTest,
	"DivineBeastsArena.GameCore.Character.BuildSummary.FixedSkillGroupIgnoresFiveCamp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterBuildSummaryFixedSkillGroupTest::RunTest(const FString& Parameters)
{
	const FDBACharacterBuildSummary EastRatWater = DBACharacterBuild::MakeBuildSummary(
		EDBAZodiac::Rat,
		EDBAElement::Water,
		EDBAFiveCamp::East);

	const FDBACharacterBuildSummary NorthRatWater = DBACharacterBuild::MakeBuildSummary(
		EDBAZodiac::Rat,
		EDBAElement::Water,
		EDBAFiveCamp::North);

	TestTrue(TEXT("鼠水东营应生成有效构筑摘要"), EastRatWater.IsValid());
	TestTrue(TEXT("鼠水北营应生成有效构筑摘要"), NorthRatWater.IsValid());
	TestEqual(TEXT("固定技能组标识应只依赖生肖和元素"), EastRatWater.FixedSkillGroupId, FName(TEXT("Rat_Water")));
	TestEqual(TEXT("改变五营不应改变固定技能组标识"), NorthRatWater.FixedSkillGroupId, EastRatWater.FixedSkillGroupId);
	TestEqual(TEXT("五营仍应作为表现选择保留"), NorthRatWater.FiveCamp, EDBAFiveCamp::North);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterBuildSummaryValidationTest,
	"DivineBeastsArena.GameCore.Character.BuildSummary.ValidatesIdentityDimensions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterBuildSummaryValidationTest::RunTest(const FString& Parameters)
{
	TestFalse(
		TEXT("缺少生肖应使构筑摘要无效"),
		DBACharacterBuild::MakeBuildSummary(EDBAZodiac::None, EDBAElement::Fire, EDBAFiveCamp::South).IsValid());

	TestFalse(
		TEXT("缺少元素应使构筑摘要无效"),
		DBACharacterBuild::MakeBuildSummary(EDBAZodiac::Tiger, EDBAElement::None, EDBAFiveCamp::South).IsValid());

	const FDBACharacterBuildSummary AutoCamp = DBACharacterBuild::MakeBuildSummary(
		EDBAZodiac::Tiger,
		EDBAElement::Fire,
		EDBAFiveCamp::None);

	TestTrue(TEXT("缺少五营应解析为稳定表现营地"), AutoCamp.IsValid());
	TestNotEqual(TEXT("解析后的五营不应保持为空"), AutoCamp.FiveCamp, EDBAFiveCamp::None);
	TestEqual(TEXT("自动五营不应影响固定技能组标识"), AutoCamp.FixedSkillGroupId, FName(TEXT("Tiger_Fire")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBATravelContextBuildSummaryValidationTest,
	"DivineBeastsArena.GameCore.Character.BuildSummary.TravelContextRejectsTamperedFixedSkillGroup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBATravelContextBuildSummaryValidationTest::RunTest(const FString& Parameters)
{
	FDBATravelContext Context;
	Context.SelectedZodiac = EDBAZodiac::Rat;
	Context.SelectedElement = EDBAElement::Water;
	Context.SelectedFiveCamp = EDBAFiveCamp::East;
	Context.FixedSkillGroupId = FName(TEXT("Rat_Water"));

	TestTrue(TEXT("旅行上下文应接受匹配的冻结构筑摘要"), Context.HasValidCharacterBuildSummary());

	const FDBACharacterBuildSummary Summary = Context.GetCharacterBuildSummary();
	TestEqual(TEXT("旅行上下文摘要应保留生肖"), Summary.Zodiac, EDBAZodiac::Rat);
	TestEqual(TEXT("旅行上下文摘要应保留元素"), Summary.PrimaryElement, EDBAElement::Water);
	TestEqual(TEXT("旅行上下文摘要应保留五营"), Summary.FiveCamp, EDBAFiveCamp::East);
	TestEqual(TEXT("旅行上下文摘要应保留固定技能组标识"), Summary.FixedSkillGroupId, FName(TEXT("Rat_Water")));

	Context.SelectedFiveCamp = EDBAFiveCamp::North;
	TestTrue(TEXT("改变五营不应使同一生肖元素技能组无效"), Context.HasValidCharacterBuildSummary());

	Context.FixedSkillGroupId = FName(TEXT("Rat_Fire"));
	TestFalse(TEXT("旅行上下文应拒绝被篡改的固定技能组标识"), Context.HasValidCharacterBuildSummary());

	Context.FixedSkillGroupId = FName(TEXT("Rat_Water"));
	Context.SelectedElement = EDBAElement::None;
	TestFalse(TEXT("旅行上下文应拒绝缺少元素但存在固定技能组标识的摘要"), Context.HasValidCharacterBuildSummary());
	return true;
}

#endif
