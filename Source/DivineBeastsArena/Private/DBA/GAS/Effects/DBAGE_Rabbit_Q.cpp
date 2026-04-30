// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - 兔Q技能

#include "DBA/GAS/Effects/DBAGE_Rabbit_Q.h"

UDBAGE_Rabbit_Q::UDBAGE_Rabbit_Q()
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
