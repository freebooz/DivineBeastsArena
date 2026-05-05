// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 猴E技能 技能类实现

#include "DBAGameplayAbility_Monkey_E.h"

UDBAGameplayAbility_Monkey_E::UDBAGameplayAbility_Monkey_E()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Monkey.E"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
