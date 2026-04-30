// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - 猪E技能

#include "DBA/GAS/Effects/DBAGE_Pig_E.h"

UDBAGE_Pig_E::UDBAGE_Pig_E()
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
