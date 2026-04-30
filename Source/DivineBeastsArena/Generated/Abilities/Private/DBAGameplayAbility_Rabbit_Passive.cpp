// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 兔被动技能 技能类实现

#include "DBAGameplayAbility_Rabbit_Passive.h"

UDBAGameplayAbility_Rabbit_Passive::UDBAGameplayAbility_Rabbit_Passive()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Rabbit.Passive"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Rabbit;
}
