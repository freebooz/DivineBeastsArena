// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端自动化测试。
- 文件职责：验证 Arena HUD EventFeed Controller 能缓存最近事件，避免 Root HUD 后绑定时丢失关键反馈。
- 阅读重点：测试只覆盖 C++ Controller 状态，不依赖 UMG 资产、关卡或真实 GAS。
- 修改提示：保持事件反馈与具体蓝图表现解耦。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/UI/Arena/UDBAArenaHUDWidgetController.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAArenaHUDEventFeedCacheTest,
	"DivineBeastsArena.UI.ArenaHUD.EventFeedCachesLatestEntry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAArenaHUDEventFeedCacheTest::RunTest(const FString& Parameters)
{
	UDBAArenaHUDWidgetController* Controller = NewObject<UDBAArenaHUDWidgetController>();
	TestNotNull(TEXT("控制器应成功创建"), Controller);

	FDBAArenaEventFeedEntry Entry = Controller->GetLastEventFeedEntry();
	TestFalse(TEXT("事件流初始不应有缓存条目"), Entry.bIsValid);

	Controller->AddEventFeedEntry(FText::FromString(TEXT("  技能命中已确认  ")), -3.0f);

	Entry = Controller->GetLastEventFeedEntry();
	TestTrue(TEXT("事件流应缓存最新条目"), Entry.bIsValid);
	TestEqual(TEXT("缓存事件文本应匹配"), Entry.Text.ToString(), FString(TEXT("技能命中已确认")));
	TestEqual(TEXT("缓存事件持续时间应被裁剪"), Entry.Duration, 0.0f);

	Controller->AddEventFeedEntry(FText::FromString(TEXT("   ")), 4.0f);

	Entry = Controller->GetLastEventFeedEntry();
	TestTrue(TEXT("空白事件流条目应被忽略"), Entry.bIsValid);
	TestEqual(TEXT("空白事件流条目应保留缓存文本"), Entry.Text.ToString(), FString(TEXT("技能命中已确认")));
	TestEqual(TEXT("空白事件流条目应保留缓存持续时间"), Entry.Duration, 0.0f);

	Controller->ClearEventFeed();

	Entry = Controller->GetLastEventFeedEntry();
	TestFalse(TEXT("清空事件流应重置缓存条目"), Entry.bIsValid);
	return true;
}

#endif
