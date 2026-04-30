// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 鼠被动技能 技能类实现

#include "DBAGameplayAbility_Rat_Passive.h"

UDBAGameplayAbility_Rat_Passive::UDBAGameplayAbility_Rat_Passive()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Rat.Passive"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Rat;
}
