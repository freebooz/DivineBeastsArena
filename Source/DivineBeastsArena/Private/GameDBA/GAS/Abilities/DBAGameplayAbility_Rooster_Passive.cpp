// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 鸡被动技能 技能类实现

#include "DBAGameplayAbility_Rooster_Passive.h"

UDBAGameplayAbility_Rooster_Passive::UDBAGameplayAbility_Rooster_Passive()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Rooster.Passive"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Rooster;
}
