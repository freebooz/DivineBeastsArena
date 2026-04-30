// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - 蛇被动技能

#include "DBA/GAS/Effects/DBAGE_Snake_Passive.h"

UDBAGE_Snake_Passive::UDBAGE_Snake_Passive()
{
	// 技能效果配置
	// DurationPolicy = EGameplayEffectDurationType::Instant;

	// 示例: 伤害修饰
	// FGameplayModifierInfo DamageMod;
	// DamageMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
	// DamageMod.ModifierOp = EGameplayModOp::Additive;
	// DamageMod.ModifierMagnitude = FScalableFloat(10.0f);
	// Modifiers.Add(DamageMod);
}
