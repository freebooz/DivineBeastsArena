// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 马终极技能 技能类实现

#include "DBAGameplayAbility_Horse_R.h"

UDBAGameplayAbility_Horse_R::UDBAGameplayAbility_Horse_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Horse.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Horse;
}
