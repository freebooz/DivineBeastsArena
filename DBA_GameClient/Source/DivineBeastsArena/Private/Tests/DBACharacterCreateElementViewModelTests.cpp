// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementViewModel.h"
#include "Misc/AutomationTest.h"

/**
 * 元素第二步的显示契约：固定构筑只允许规则行投影为只读技能 ID 列表，
 * 不提供自由增删技能的入口。依据仓库人工审核策略，本文件不在本步骤自动执行。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterCreateElementViewModelTest,
	"DBA.Frontend.CharacterCreate.Element.FixedBuildProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterCreateElementViewModelTest::RunTest(const FString& Parameters)
{
	UDBACharacterCreateElementViewModel* ViewModel = NewObject<UDBACharacterCreateElementViewModel>();
	FDBACharacterCreateDraft Draft;
	Draft.ZodiacType = EDBAZodiac::Rat;
	Draft.ElementType = EDBAElement::Water;
	ViewModel->ApplyDraft(Draft);

	FDBAZodiacElementFixedSkillGroupRow RuleRow;
	RuleRow.RowId = TEXT("Rat_Water");
	RuleRow.ZodiacType = EDBAZodiac::Rat;
	RuleRow.ElementType = EDBAElement::Water;
	RuleRow.ElementPassiveSkillId = TEXT("Rat_Water_Passive");
	RuleRow.ElementSkill1Id = TEXT("Rat_Water_Skill01");
	RuleRow.ZodiacUltimateSkillId = TEXT("Rat_Ultimate");
	RuleRow.ElementResonanceLevel = 4;
	RuleRow.ResonanceElement = EDBAElement::Water;
	RuleRow.bEnabled = true;
	RuleRow.bIsInDevelopment = false;
	ViewModel->ApplyFixedSkillBuild(&RuleRow);

	const FDBAFixedSkillBuildPreviewModel& Preview = ViewModel->GetFixedSkillBuildPreview();
	TestEqual(TEXT("固定构筑必须保留规则行标识"), Preview.FixedSkillBuildRowId, FName(TEXT("Rat_Water")));
	TestEqual(TEXT("技能列表只能由规则行投影生成"), Preview.SkillIds.Num(), 3);
	TestTrue(TEXT("有效规则行才能标记为可展示构筑"), Preview.bIsReady);
	TestEqual(TEXT("元素选择投影来自 Draft"), ViewModel->GetSelectedElement(), EDBAElement::Water);
	return true;
}

#endif
