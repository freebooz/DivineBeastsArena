// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 马被动技能 技能类实现

#include "DBAGameplayAbility_Horse_Passive.h"

UDBAGameplayAbility_Horse_Passive::UDBAGameplayAbility_Horse_Passive()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Horse.Passive"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Horse;
}
