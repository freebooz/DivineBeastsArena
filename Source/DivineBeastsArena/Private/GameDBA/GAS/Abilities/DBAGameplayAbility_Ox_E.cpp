// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 牛E技能 技能类实现

#include "DBAGameplayAbility_Ox_E.h"

UDBAGameplayAbility_Ox_E::UDBAGameplayAbility_Ox_E()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Ox.E"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
