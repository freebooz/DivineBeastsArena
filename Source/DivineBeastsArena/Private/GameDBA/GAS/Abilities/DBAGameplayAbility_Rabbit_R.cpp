// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 兔终极技能 技能类实现

#include "DBAGameplayAbility_Rabbit_R.h"

UDBAGameplayAbility_Rabbit_R::UDBAGameplayAbility_Rabbit_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Rabbit.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Rabbit;
}
