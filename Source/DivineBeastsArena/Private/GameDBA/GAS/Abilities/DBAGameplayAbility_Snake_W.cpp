// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 蛇W技能 技能类实现

#include "DBAGameplayAbility_Snake_W.h"

UDBAGameplayAbility_Snake_W::UDBAGameplayAbility_Snake_W()
{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Snake.W"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
	// ElementType 由英雄实例在 SpawnAbility 时设置
}
