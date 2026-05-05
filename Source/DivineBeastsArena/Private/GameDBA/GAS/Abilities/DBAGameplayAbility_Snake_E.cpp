// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 蛇E技能 技能类实现

#include "DBAGameplayAbility_Snake_E.h"

UDBAGameplayAbility_Snake_E::UDBAGameplayAbility_Snake_E()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Snake.E"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
