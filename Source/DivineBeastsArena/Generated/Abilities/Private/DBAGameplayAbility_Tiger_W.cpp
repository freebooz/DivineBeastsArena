// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 虎W技能 技能类实现

#include "DBAGameplayAbility_Tiger_W.h"

UDBAGameplayAbility_Tiger_W::UDBAGameplayAbility_Tiger_W()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Tiger.W"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
