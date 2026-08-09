// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateZodiacViewModel.h"
#include "Misc/AutomationTest.h"

/**
 * 第一创建步骤的显示契约测试：验证列表仅接受 Registry 枚举结果，
 * 且 Draft 投影只改变选中态，不让 Widget 私自保存第二份创建业务状态。
 * 本文件仅提供工程契约；依据仓库人工审核策略，本步骤未自动执行它。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterCreateZodiacViewModelTest,
	"DBA.Frontend.CharacterCreate.ZodiacAppearance.ViewModelProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterCreateZodiacViewModelTest::RunTest(const FString& Parameters)
{
	UDBACharacterCreateZodiacViewModel* ViewModel = NewObject<UDBACharacterCreateZodiacViewModel>();
	ViewModel->SetAvailableZodiacs({ EDBAZodiac::Rat, EDBAZodiac::Dragon });

	TestEqual(TEXT("生肖列表必须来自 Registry 的实际枚举结果"), ViewModel->GetZodiacItems().Num(), 2);
	TestEqual(TEXT("第一项保留 Registry 返回的生肖标识"), ViewModel->GetZodiacItems()[0].Zodiac, EDBAZodiac::Rat);

	FDBACharacterCreateDraft Draft;
	Draft.ZodiacType = EDBAZodiac::Dragon;
	Draft.Appearance.HairId = TEXT("Hair.Dragon.Default");
	ViewModel->ApplyDraft(Draft);

	TestEqual(TEXT("Draft 的生肖投影为当前选中生肖"), ViewModel->GetSelectedZodiac(), EDBAZodiac::Dragon);
	TestTrue(TEXT("只有 Draft 选中的生肖显示为选中态"), ViewModel->GetZodiacItems()[1].bIsSelected);
	TestFalse(TEXT("非选中生肖不得残留选中态"), ViewModel->GetZodiacItems()[0].bIsSelected);
	TestEqual(TEXT("外观 ViewModel 只保存稳定 OptionId"), ViewModel->GetAppearance().HairId, FName(TEXT("Hair.Dragon.Default")));
	return true;
}

#endif
