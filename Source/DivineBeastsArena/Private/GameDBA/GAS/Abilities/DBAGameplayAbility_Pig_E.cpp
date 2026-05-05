// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 猪E技能 技能类实现

#include "DBAGameplayAbility_Pig_E.h"

UDBAGameplayAbility_Pig_E::UDBAGameplayAbility_Pig_E()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Pig.E"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
