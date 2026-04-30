// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 狗Q技能 技能类实现

#include "DBAGameplayAbility_Dog_Q.h"

UDBAGameplayAbility_Dog_Q::UDBAGameplayAbility_Dog_Q()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dog.Q"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
