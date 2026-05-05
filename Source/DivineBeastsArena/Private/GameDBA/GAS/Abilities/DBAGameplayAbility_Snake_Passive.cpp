// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 蛇被动技能 技能类实现

#include "DBAGameplayAbility_Snake_Passive.h"

UDBAGameplayAbility_Snake_Passive::UDBAGameplayAbility_Snake_Passive()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Snake.Passive"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	ZodiacType = EDBAZodiacType::Snake;
}
