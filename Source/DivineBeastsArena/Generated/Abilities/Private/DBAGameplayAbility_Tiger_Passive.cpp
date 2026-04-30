// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 虎被动技能 技能类实现

#include "DBAGameplayAbility_Tiger_Passive.h"

UDBAGameplayAbility_Tiger_Passive::UDBAGameplayAbility_Tiger_Passive()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Tiger.Passive"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Tiger;
}
