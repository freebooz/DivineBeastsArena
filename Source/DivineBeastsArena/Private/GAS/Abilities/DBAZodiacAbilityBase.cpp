// Copyright FreeboozStudio. All Rights Reserved.

#include "GAS/Abilities/DBAZodiacAbilityBase.h"

UDBAZodiacAbilityBase::UDBAZodiacAbilityBase()
{
	// 默认生肖类型为鼠（生肖顺序第一位）
	ZodiacType = EDBAZodiacType::Rat;
}

bool UDBAZodiacAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 调用父类检查（BlockTags、冷却等）
	// 生肖技能通常为被动技能，激活逻辑复用基类
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}
