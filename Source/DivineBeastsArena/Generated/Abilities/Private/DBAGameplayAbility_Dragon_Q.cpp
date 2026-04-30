// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 龙Q技能 技能类实现

#include "DBAGameplayAbility_Dragon_Q.h"

UDBAGameplayAbility_Dragon_Q::UDBAGameplayAbility_Dragon_Q()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dragon.Q"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
