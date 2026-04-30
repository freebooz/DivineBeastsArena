// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 狗被动技能 技能类实现

#include "DBAGameplayAbility_Dog_Passive.h"

UDBAGameplayAbility_Dog_Passive::UDBAGameplayAbility_Dog_Passive()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dog.Passive"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Dog;
}
