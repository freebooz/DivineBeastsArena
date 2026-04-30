// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - 鸡被动技能

#include "DBA/GAS/Effects/DBAGE_Rooster_Passive.h"

UDBAGE_Rooster_Passive::UDBAGE_Rooster_Passive()
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
