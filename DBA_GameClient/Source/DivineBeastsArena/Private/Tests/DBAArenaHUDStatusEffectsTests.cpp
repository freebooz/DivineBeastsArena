// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端自动化测试。
- 文件职责：验证 Arena HUD StatusEffects Controller 能缓存活跃 Buff/Debuff/CC，避免 Root HUD 后绑定时丢失状态栏信息。
- 阅读重点：测试只覆盖 C++ Controller 状态，不依赖 UMG 资产、关卡或真实 GAS。
- 修改提示：保持状态效果反馈与具体蓝图表现解耦。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/UI/Arena/UDBAArenaHUDWidgetController.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAArenaHUDStatusEffectsCacheTest,
	"DivineBeastsArena.UI.ArenaHUD.StatusEffectsCacheActiveEntries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAArenaHUDStatusEffectsCacheTest::RunTest(const FString& Parameters)
{
	UDBAArenaHUDWidgetController* Controller = NewObject<UDBAArenaHUDWidgetController>();
	TestNotNull(TEXT("竞技场界面控制器应能创建"), Controller);

	TestEqual(TEXT("增益缓存初始应为空"), Controller->GetActiveStatusBuffs().Num(), 0);
	TestEqual(TEXT("减益缓存初始应为空"), Controller->GetActiveStatusDebuffs().Num(), 0);
	TestEqual(TEXT("控制效果缓存初始应为空"), Controller->GetActiveStatusCCEffects().Num(), 0);

	Controller->AddStatusBuff(TEXT("Haste"), -1.0f);
	TArray<FDBAArenaStatusEffectEntry> Buffs = Controller->GetActiveStatusBuffs();
	TestEqual(TEXT("添加增益应缓存一条记录"), Buffs.Num(), 1);
	TestEqual(TEXT("增益标识应被缓存"), Buffs[0].EffectId, FString(TEXT("Haste")));
	TestEqual(TEXT("增益持续时间应被裁剪"), Buffs[0].Duration, 0.0f);

	Controller->AddStatusBuff(TEXT("Haste"), 3.5f);
	Buffs = Controller->GetActiveStatusBuffs();
	TestEqual(TEXT("重复添加同标识增益应更新现有记录"), Buffs.Num(), 1);
	TestEqual(TEXT("增益持续时间应更新"), Buffs[0].Duration, 3.5f);

	Controller->RemoveStatusBuff(TEXT("Haste"));
	TestEqual(TEXT("移除增益应更新缓存"), Controller->GetActiveStatusBuffs().Num(), 0);

	Controller->AddStatusDebuff(TEXT("Burn"), 2.0f);
	TArray<FDBAArenaStatusEffectEntry> Debuffs = Controller->GetActiveStatusDebuffs();
	TestEqual(TEXT("添加减益应缓存一条记录"), Debuffs.Num(), 1);
	TestEqual(TEXT("减益标识应被缓存"), Debuffs[0].EffectId, FString(TEXT("Burn")));
	TestEqual(TEXT("减益持续时间应被缓存"), Debuffs[0].Duration, 2.0f);
	Controller->ClearStatusDebuffs();
	TestEqual(TEXT("清空减益应更新缓存"), Controller->GetActiveStatusDebuffs().Num(), 0);

	Controller->AddStatusCCEffect(TEXT("Stun"), 1.25f);
	TArray<FDBAArenaStatusEffectEntry> CCEffects = Controller->GetActiveStatusCCEffects();
	TestEqual(TEXT("添加控制效果应缓存一条记录"), CCEffects.Num(), 1);
	TestEqual(TEXT("控制效果标识应被缓存"), CCEffects[0].EffectId, FString(TEXT("Stun")));
	TestEqual(TEXT("控制效果持续时间应被缓存"), CCEffects[0].Duration, 1.25f);
	Controller->RemoveStatusCCEffect(TEXT("Stun"));
	TestEqual(TEXT("移除控制效果应更新缓存"), Controller->GetActiveStatusCCEffects().Num(), 0);
	return true;
}

#endif
