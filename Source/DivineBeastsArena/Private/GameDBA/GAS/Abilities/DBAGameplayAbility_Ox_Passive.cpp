// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 牛被动技能 技能类实现

#include "DBAGameplayAbility_Ox_Passive.h"

UDBAGameplayAbility_Ox_Passive::UDBAGameplayAbility_Ox_Passive()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Ox.Passive"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Ox;
}
