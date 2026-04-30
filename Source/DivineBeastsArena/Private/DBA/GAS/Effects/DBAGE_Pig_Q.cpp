// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - 猪Q技能

#include "DBA/GAS/Effects/DBAGE_Pig_Q.h"

UDBAGE_Pig_Q::UDBAGE_Pig_Q()
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
