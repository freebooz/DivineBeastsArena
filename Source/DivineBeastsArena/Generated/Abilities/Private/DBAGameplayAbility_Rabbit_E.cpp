// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 兔E技能 技能类实现

#include "DBAGameplayAbility_Rabbit_E.h"

UDBAGameplayAbility_Rabbit_E::UDBAGameplayAbility_Rabbit_E()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Rabbit.E"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
