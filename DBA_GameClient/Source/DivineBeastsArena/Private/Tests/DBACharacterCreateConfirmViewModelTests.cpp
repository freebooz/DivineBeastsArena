// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateConfirmViewModel.h"
#include "Misc/AutomationTest.h"

/**
 * 锁定确认页只从唯一 Draft 投影摘要的契约。测试不访问网络、不创建角色、不启动前台流程，
 * 因此不能替代人工审核，但可防止未来 UI 重新持有第二份名称或选择状态。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterCreateConfirmViewModelDraftTest,
	"DBA.Frontend.CharacterCreate.Confirm.ViewModelReadsDraft",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterCreateConfirmViewModelDraftTest::RunTest(const FString& Parameters)
{
	FDBACharacterCreateDraft Draft;
	Draft.CharacterName = TEXT("确认页测试角色");
	Draft.ZodiacType = EDBAZodiac::Rat;
	Draft.ElementType = EDBAElement::Water;
	Draft.FiveCampType = EDBAFiveCamp::East;
	Draft.FixedSkillBuildRowId = TEXT("Rat_Water");
	Draft.PreviewSummary = NSLOCTEXT("DBAConfirmTest", "Preview", "固定构筑属性预览");
	Draft.Appearance.HairId = TEXT("Rat_Hair_01");

	UDBACharacterCreateConfirmViewModel* ViewModel = NewObject<UDBACharacterCreateConfirmViewModel>();
	ViewModel->ApplyDraft(Draft);

	TestEqual(TEXT("确认页角色名必须来自 Draft"), ViewModel->GetCharacterName(), Draft.CharacterName);
	TestTrue(TEXT("外观摘要必须反映已配置外观 ID"), ViewModel->GetAppearanceSummary().ToString().Contains(TEXT("1")));
	TestEqual(TEXT("属性摘要必须复用 Draft 展示摘要"), ViewModel->GetAttributeSummary().ToString(), Draft.PreviewSummary.ToString());
	return true;
}

#endif
