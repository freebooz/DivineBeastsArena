// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 猴终极技能 技能类实现

#include "DBAGameplayAbility_Monkey_R.h"

UDBAGameplayAbility_Monkey_R::UDBAGameplayAbility_Monkey_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Monkey.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Monkey;
}
