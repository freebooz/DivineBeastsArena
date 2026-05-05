// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 鸡E技能 技能类实现

#include "DBAGameplayAbility_Rooster_E.h"

UDBAGameplayAbility_Rooster_E::UDBAGameplayAbility_Rooster_E()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Rooster.E"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
