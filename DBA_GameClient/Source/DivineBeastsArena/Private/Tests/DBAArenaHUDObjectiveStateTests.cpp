// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端自动化测试。
- 文件职责：验证 Arena HUD Objective Controller 能缓存最近目标状态，避免 Root HUD 后绑定时丢失目标文本/进度。
- 阅读重点：测试只覆盖 C++ Controller 状态，不依赖 UMG 资产、关卡或真实任务系统。
- 修改提示：保持目标反馈与具体蓝图表现解耦。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/UI/Arena/UDBAArenaHUDWidgetController.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAArenaHUDObjectiveStateCacheTest,
	"DivineBeastsArena.UI.ArenaHUD.ObjectiveCachesLatestState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAArenaHUDObjectiveStateCacheTest::RunTest(const FString& Parameters)
{
	UDBAArenaHUDWidgetController* Controller = NewObject<UDBAArenaHUDWidgetController>();
	TestNotNull(TEXT("控制器应成功创建"), Controller);

	FDBAArenaObjectiveState State = Controller->GetLastArenaObjectiveState();
	TestFalse(TEXT("目标状态初始应无效"), State.bIsValid);

	Controller->UpdateArenaObjective(FText::FromString(TEXT("  占领祭坛  ")), 1.5f);

	State = Controller->GetLastArenaObjectiveState();
	TestTrue(TEXT("目标状态应缓存最新更新"), State.bIsValid);
	TestFalse(TEXT("目标状态更新时不应完成"), State.bIsCompleted);
	TestEqual(TEXT("目标文本应匹配"), State.ObjectiveText.ToString(), FString(TEXT("占领祭坛")));
	TestEqual(TEXT("目标进度应被裁剪"), State.Progress, 1.0f);

	Controller->UpdateArenaObjective(FText::FromString(TEXT("   ")), 0.25f);

	State = Controller->GetLastArenaObjectiveState();
	TestTrue(TEXT("空白目标更新应被忽略"), State.bIsValid);
	TestFalse(TEXT("空白目标更新应保留完成标记"), State.bIsCompleted);
	TestEqual(TEXT("空白目标更新应保留文本"), State.ObjectiveText.ToString(), FString(TEXT("占领祭坛")));
	TestEqual(TEXT("空白目标更新应保留进度"), State.Progress, 1.0f);

	Controller->CompleteArenaObjective();

	State = Controller->GetLastArenaObjectiveState();
	TestTrue(TEXT("目标状态完成后仍应有效"), State.bIsValid);
	TestTrue(TEXT("目标状态应缓存完成标记"), State.bIsCompleted);
	TestEqual(TEXT("目标完成应强制满进度"), State.Progress, 1.0f);
	return true;
}

#endif
