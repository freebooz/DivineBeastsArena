// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 羊被动技能 技能类实现

#include "DBAGameplayAbility_Goat_Passive.h"

UDBAGameplayAbility_Goat_Passive::UDBAGameplayAbility_Goat_Passive()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Goat.Passive"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Goat;
}
