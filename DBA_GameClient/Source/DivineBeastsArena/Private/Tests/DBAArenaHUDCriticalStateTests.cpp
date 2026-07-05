// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端自动化测试。
- 文件职责：验证 Arena HUD CriticalState Controller 能缓存最近低血/低能量状态，避免 Root HUD 后绑定时丢失危急提示。
- 阅读重点：测试只覆盖 C++ Controller 状态，不依赖 UMG 资产、关卡或真实角色属性。
- 修改提示：保持危急提示反馈与具体蓝图表现解耦。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/UI/Arena/UDBAArenaHUDWidgetController.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAArenaHUDCriticalStateCacheTest,
	"DivineBeastsArena.UI.ArenaHUD.CriticalStateCachesLatestState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAArenaHUDCriticalStateCacheTest::RunTest(const FString& Parameters)
{
	UDBAArenaHUDWidgetController* Controller = NewObject<UDBAArenaHUDWidgetController>();
	TestNotNull(TEXT("竞技场界面控制器应能创建"), Controller);

	FDBAArenaCriticalStateHintState State = Controller->GetLastCriticalStateHints();
	TestFalse(TEXT("危急状态初始应无效"), State.bIsValid);

	Controller->UpdateCriticalStateHints(true, false);

	State = Controller->GetLastCriticalStateHints();
	TestTrue(TEXT("危急状态应缓存最新更新"), State.bIsValid);
	TestTrue(TEXT("危急状态应缓存低血量标记"), State.bLowHP);
	TestFalse(TEXT("危急状态应缓存能量正常标记"), State.bLowEnergy);

	Controller->UpdateCriticalStateHints(false, false);

	State = Controller->GetLastCriticalStateHints();
	TestTrue(TEXT("危急状态重置后仍应有效"), State.bIsValid);
	TestFalse(TEXT("危急状态应清除低血量标记"), State.bLowHP);
	TestFalse(TEXT("危急状态应清除低能量标记"), State.bLowEnergy);
	return true;
}

#endif
