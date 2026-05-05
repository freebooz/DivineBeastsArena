// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 猪被动技能 技能类实现

#include "DBAGameplayAbility_Pig_Passive.h"

UDBAGameplayAbility_Pig_Passive::UDBAGameplayAbility_Pig_Passive()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Pig.Passive"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Pig;
}
