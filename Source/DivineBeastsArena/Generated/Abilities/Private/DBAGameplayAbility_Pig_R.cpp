// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 猪终极技能 技能类实现

#include "DBAGameplayAbility_Pig_R.h"

UDBAGameplayAbility_Pig_R::UDBAGameplayAbility_Pig_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Pig.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Pig;
}
