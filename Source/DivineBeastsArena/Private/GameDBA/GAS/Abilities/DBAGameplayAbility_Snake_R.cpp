// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 蛇终极技能 技能类实现

#include "DBAGameplayAbility_Snake_R.h"

UDBAGameplayAbility_Snake_R::UDBAGameplayAbility_Snake_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Snake.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Snake;
}
