// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateFiveCampViewModel.h"
#include "Misc/AutomationTest.h"

/**
 * 验证五营页面只以 Draft 为选中态来源。测试不涉及网络、PreviewStage、TeamId 或 UMG，
 * 仅锁定 ViewModel 投影契约，避免后续布局改动重新引入 Widget 本地业务状态。
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBACharacterCreateFiveCampViewModelSelectionTest,
	"DBA.Frontend.CharacterCreate.FiveCamp.ViewModelSelection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterCreateFiveCampViewModelSelectionTest::RunTest(const FString& Parameters)
{
	// 构造数据表加载完成后 Controller 会传入的两个只读卡片投影；不在测试中硬编码任何展示资源或业务属性。
	FDBACharacterCreateFiveCampCardModel EastCard;
	EastCard.FiveCamp = EDBAFiveCamp::East;
	EastCard.bIsAvailable = true;
	FDBACharacterCreateFiveCampCardModel WestCard;
	WestCard.FiveCamp = EDBAFiveCamp::West;
	WestCard.bIsAvailable = true;

	UDBACharacterCreateFiveCampViewModel* ViewModel = NewObject<UDBACharacterCreateFiveCampViewModel>();
	ViewModel->ApplyFiveCampCards({ EastCard, WestCard });

	FDBACharacterCreateDraft Draft;
	Draft.FiveCampType = EDBAFiveCamp::West;
	ViewModel->ApplyDraft(Draft);

	TestEqual(TEXT("选中态必须来自唯一 Draft 的 FiveCamp 值"), ViewModel->GetSelectedFiveCamp(), EDBAFiveCamp::West);
	TestEqual(TEXT("当前详情卡片必须与 Draft 选择一致"), ViewModel->GetSelectedCard().FiveCamp, EDBAFiveCamp::West);
	TestTrue(TEXT("未选中的卡片不得保留选中态"), !ViewModel->GetFiveCampCards()[0].bIsSelected);
	TestTrue(TEXT("选中的卡片必须标记选中态"), ViewModel->GetFiveCampCards()[1].bIsSelected);
	return true;
}

#endif
