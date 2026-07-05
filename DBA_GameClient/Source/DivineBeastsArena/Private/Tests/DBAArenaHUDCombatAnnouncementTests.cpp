// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端自动化测试。
- 文件职责：验证 Arena HUD CombatAnnouncement Controller 能缓存最近公告，避免 Root HUD 后绑定时丢失战斗提示。
- 阅读重点：测试只覆盖 C++ Controller 状态，不依赖 UMG 资产、关卡或真实 GAS。
- 修改提示：保持公告反馈与具体蓝图表现解耦。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/UI/Arena/UDBAArenaHUDWidgetController.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAArenaHUDCombatAnnouncementCacheTest,
	"DivineBeastsArena.UI.ArenaHUD.CombatAnnouncementCachesLatestEntry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAArenaHUDCombatAnnouncementCacheTest::RunTest(const FString& Parameters)
{
	UDBAArenaHUDWidgetController* Controller = NewObject<UDBAArenaHUDWidgetController>();
	TestNotNull(TEXT("控制器应成功创建"), Controller);

	FDBAArenaCombatAnnouncementEntry Entry = Controller->GetLastCombatAnnouncement();
	TestFalse(TEXT("战斗公告初始不应有缓存条目"), Entry.bIsValid);

	Controller->ShowCombatAnnouncement(FText::FromString(TEXT("  连锁已就绪  ")), -2.0f);

	Entry = Controller->GetLastCombatAnnouncement();
	TestTrue(TEXT("战斗公告应缓存最新条目"), Entry.bIsValid);
	TestEqual(TEXT("缓存公告文本应匹配"), Entry.Text.ToString(), FString(TEXT("连锁已就绪")));
	TestEqual(TEXT("缓存公告持续时间应被裁剪"), Entry.Duration, 0.0f);

	Controller->ShowCombatAnnouncement(FText::FromString(TEXT("   ")), 5.0f);

	Entry = Controller->GetLastCombatAnnouncement();
	TestTrue(TEXT("空白战斗公告应被忽略"), Entry.bIsValid);
	TestEqual(TEXT("空白战斗公告应保留缓存文本"), Entry.Text.ToString(), FString(TEXT("连锁已就绪")));
	TestEqual(TEXT("空白战斗公告应保留缓存持续时间"), Entry.Duration, 0.0f);

	Controller->ClearCombatAnnouncement();

	Entry = Controller->GetLastCombatAnnouncement();
	TestFalse(TEXT("清空战斗公告应重置缓存条目"), Entry.bIsValid);
	return true;
}

#endif
