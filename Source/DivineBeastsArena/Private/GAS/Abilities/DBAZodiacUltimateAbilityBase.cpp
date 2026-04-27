// Copyright FreeboozStudio. All Rights Reserved.
// 生肖终极能力基类实现 - 生肖大招能力的基类

#include "GAS/Abilities/DBAZodiacUltimateAbilityBase.h"
#include "GAS/DBAAbilitySystemComponent.h"

// 构造函数 - 初始化生肖终极能力
UDBAZodiacUltimateAbilityBase::UDBAZodiacUltimateAbilityBase()
{
	// 生肖大招默认配置
}

// CanActivateAbility - 检查能力是否可以激活
bool UDBAZodiacUltimateAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 调用父类检查
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 获取能力系统组件
	UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (ASC)
	{
		// 检查终极能量是否满 100
		return ASC->HasEnoughUltimateEnergy(100.0f);
	}

	return false;
}

// CommitAbilityCost - 提交能力消耗（终极能量）
bool UDBAZodiacUltimateAbilityBase::CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	// 调用父类提交消耗
	if (!Super::CommitAbilityCost(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	// 仅服务端权威扣除 UltimateEnergy
	UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (ASC && ActorInfo->IsNetAuthority())
	{
		// 消耗100点终极能量
		ASC->ConsumeUltimateEnergy(100.0f);
	}

	return true;
}