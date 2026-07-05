// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端自动化测试。
- 文件职责：验证 Arena HUD UltimateReadyPrompt Controller 能缓存最近显示/隐藏状态，避免 Root HUD 后绑定时丢失大招就绪提示。
- 阅读重点：测试只覆盖 C++ Controller 状态，不依赖 UMG 资产、关卡或真实 GAS。
- 修改提示：保持大招就绪提示反馈与具体蓝图表现解耦。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/UI/Arena/UDBAArenaHUDWidgetController.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAArenaHUDUltimateReadyPromptCacheTest,
	"DivineBeastsArena.UI.ArenaHUD.UltimateReadyPromptCachesLatestState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAArenaHUDUltimateReadyPromptCacheTest::RunTest(const FString& Parameters)
{
	UDBAArenaHUDWidgetController* Controller = NewObject<UDBAArenaHUDWidgetController>();
	TestNotNull(TEXT("竞技场界面控制器应能创建"), Controller);

	FDBAArenaUltimateReadyPromptState State = Controller->GetLastUltimateReadyPromptState();
	TestFalse(TEXT("大招就绪提示初始应无效"), State.bIsValid);

	Controller->ShowUltimateReadyPrompt();

	State = Controller->GetLastUltimateReadyPromptState();
	TestTrue(TEXT("大招就绪提示应缓存最新显示状态"), State.bIsValid);
	TestTrue(TEXT("大招就绪提示应缓存可见状态"), State.bIsShown);

	Controller->HideUltimateReadyPrompt();

	State = Controller->GetLastUltimateReadyPromptState();
	TestTrue(TEXT("大招就绪提示隐藏后仍应有效"), State.bIsValid);
	TestFalse(TEXT("大招就绪提示应缓存隐藏状态"), State.bIsShown);
	return true;
}

#endif
