// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 龙被动技能 技能类实现

#include "DBAGameplayAbility_Dragon_Passive.h"

UDBAGameplayAbility_Dragon_Passive::UDBAGameplayAbility_Dragon_Passive()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dragon.Passive"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Dragon;
}
