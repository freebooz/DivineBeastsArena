// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 牛终极技能 技能类实现

#include "DBAGameplayAbility_Ox_R.h"

UDBAGameplayAbility_Ox_R::UDBAGameplayAbility_Ox_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Ox.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Ox;
}
