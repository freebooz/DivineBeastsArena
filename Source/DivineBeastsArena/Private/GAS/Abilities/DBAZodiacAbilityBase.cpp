// Copyright FreeboozStudio. All Rights Reserved.
// 生肖能力基类实现 - 所有生肖相关能力的基类

#include "GAS/Abilities/DBAZodiacAbilityBase.h"

// 构造函数 - 初始化生肖能力默认属性
UDBAZodiacAbilityBase::UDBAZodiacAbilityBase()
{
	// 默认生肖类型为鼠（生肖顺序第一位）
	ZodiacType = EDBAZodiacType::Rat;
}

// CanActivateAbility - 检查能力是否可以激活
bool UDBAZodiacAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 调用父类检查（BlockTags、冷却等）
	// 生肖技能通常为被动技能，激活逻辑复用基类
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}