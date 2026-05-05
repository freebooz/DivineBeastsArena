// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 鼠终极技能 技能类实现

#include "DBAGameplayAbility_Rat_R.h"

UDBAGameplayAbility_Rat_R::UDBAGameplayAbility_Rat_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Rat.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Rat;
}
