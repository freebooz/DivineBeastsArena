// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 虎终极技能 技能类实现

#include "DBAGameplayAbility_Tiger_R.h"

UDBAGameplayAbility_Tiger_R::UDBAGameplayAbility_Tiger_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Tiger.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Tiger;
}
