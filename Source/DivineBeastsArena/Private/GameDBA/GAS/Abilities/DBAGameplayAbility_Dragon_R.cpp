// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 龙终极技能 技能类实现

#include "DBAGameplayAbility_Dragon_R.h"

UDBAGameplayAbility_Dragon_R::UDBAGameplayAbility_Dragon_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dragon.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Dragon;
}
