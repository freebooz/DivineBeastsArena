// Copyright Freebooz Games, Inc. All Rights Reserved.
// 冷却效果实现

#include "GameDBA/GAS/Effects/DBAGE_Cooldown.h"

UDBAGE_Cooldown::UDBAGE_Cooldown()
{
	// 设置冷却效果为持续时间类型
	FGameplayEffectDurationModifier DurationPolicy;
	DurationPolicy.SetMagnitude(CooldownDuration, EGameplayEffectDurationType::HasDuration);

	bSuppressWarnings = true; // 抑制重复Tag警告

	// 冷却期间不允许相同技能激活
	// 设置为 Infinite 持续，通过 GE 结束移除冷却 Tag
	SetDurationPolicy(EGameplayEffectDurationType::Infinite);
}