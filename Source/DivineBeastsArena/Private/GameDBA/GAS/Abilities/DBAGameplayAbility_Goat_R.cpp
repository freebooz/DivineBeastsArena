// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 羊终极技能 技能类实现

#include "DBAGameplayAbility_Goat_R.h"

UDBAGameplayAbility_Goat_R::UDBAGameplayAbility_Goat_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Goat.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Goat;
}
