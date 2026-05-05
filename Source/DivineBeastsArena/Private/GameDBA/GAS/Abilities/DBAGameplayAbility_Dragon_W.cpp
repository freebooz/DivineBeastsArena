// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 龙W技能 技能类实现

#include "DBAGameplayAbility_Dragon_W.h"

UDBAGameplayAbility_Dragon_W::UDBAGameplayAbility_Dragon_W()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dragon.W"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
