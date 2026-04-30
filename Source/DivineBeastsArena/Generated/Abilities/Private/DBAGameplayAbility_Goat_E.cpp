// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 羊E技能 技能类实现

#include "DBAGameplayAbility_Goat_E.h"

UDBAGameplayAbility_Goat_E::UDBAGameplayAbility_Goat_E()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Goat.E"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
