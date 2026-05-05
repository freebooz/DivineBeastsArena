// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 狗终极技能 技能类实现

#include "DBAGameplayAbility_Dog_R.h"

UDBAGameplayAbility_Dog_R::UDBAGameplayAbility_Dog_R()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dog.R"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Dog;
}
