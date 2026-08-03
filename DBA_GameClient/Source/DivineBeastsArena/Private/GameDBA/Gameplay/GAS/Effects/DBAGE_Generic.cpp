// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 泛化GameplayEffect类实现

#include "GameDBA/Gameplay/GAS/Effects/DBAGE_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"

UDBAGE_Generic::UDBAGE_Generic()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		LoadAndApplyModifiers();
	}
}

void UDBAGE_Generic::LoadAndApplyModifiers()
{
	if (!SkillTable || SkillID.IsNone())
	{
		return;
	}

	static const FString ContextString = TEXT("DBAGE_Generic");
	FDBASkillDataRow* SkillRow = SkillTable->FindRow<FDBASkillDataRow>(SkillID, ContextString, false);

	if (!SkillRow)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAGE_Generic] 未找到技能配置: %s"), *SkillID.ToString());
		return;
	}

	// 伤害修饰符（减少生命值，使用负值 Additive）
	if (SkillRow->BaseDamage > 0)
	{
		FGameplayModifierInfo DamageMod;
		DamageMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
		DamageMod.ModifierOp = EGameplayModOp::Additive;
		DamageMod.ModifierMagnitude = FScalableFloat(-SkillRow->BaseDamage);
		Modifiers.Add(DamageMod);
	}

	// 治疗修饰符
	if (SkillRow->HealAmount > 0)
	{
		FGameplayModifierInfo HealMod;
		HealMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
		HealMod.ModifierOp = EGameplayModOp::Additive;
		HealMod.ModifierMagnitude = FScalableFloat(SkillRow->HealAmount);
		Modifiers.Add(HealMod);
	}

	// 护盾修饰符（增加到 CurrentShield，而非 CurrentHealth）
	if (SkillRow->ShieldValue > 0)
	{
		FGameplayModifierInfo ShieldMod;
		ShieldMod.Attribute = UDBABattleAttributeSet::GetCurrentShieldAttribute();
		ShieldMod.ModifierOp = EGameplayModOp::Additive;
		ShieldMod.ModifierMagnitude = FScalableFloat(SkillRow->ShieldValue);
		Modifiers.Add(ShieldMod);
	}

	// 设置持续时间
	if (SkillRow->ControlTime > 0)
	{
		DurationPolicy = EGameplayEffectDurationType::HasDuration;
		Period = 0.0f;
		DurationMagnitude = FScalableFloat(SkillRow->ControlTime);
	}

	UE_LOG(LogDBACombat, Log, TEXT("[DBAGE_Generic] 已加载技能配置：技能=%s 基础伤害=%.1f 治疗量=%.1f 护盾值=%.1f"),
		*SkillID.ToString(), SkillRow->BaseDamage, SkillRow->HealAmount, SkillRow->ShieldValue);
}
