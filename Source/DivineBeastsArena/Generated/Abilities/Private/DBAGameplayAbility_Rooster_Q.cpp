// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 鸡Q技能 技能类实现

#include "DBAGameplayAbility_Rooster_Q.h"

UDBAGameplayAbility_Rooster_Q::UDBAGameplayAbility_Rooster_Q()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Rooster.Q"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
