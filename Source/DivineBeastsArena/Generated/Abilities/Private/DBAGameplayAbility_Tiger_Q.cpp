// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 虎Q技能 技能类实现

#include "DBAGameplayAbility_Tiger_Q.h"

UDBAGameplayAbility_Tiger_Q::UDBAGameplayAbility_Tiger_Q()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Tiger.Q"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
