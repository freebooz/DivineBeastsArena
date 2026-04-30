// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 鸡终极技能 技能类实现

#include "DBAGameplayAbility_Rooster_R.h"

UDBAGameplayAbility_Rooster_R::UDBAGameplayAbility_Rooster_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Rooster.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Rooster;
}
