// Copyright FreeboozStudio. All Rights Reserved.

#include "GAS/Abilities/DBAZodiacUltimateAbilityBase.h"
#include "GAS/DBAAbilitySystemComponent.h"

UDBAZodiacUltimateAbilityBase::UDBAZodiacUltimateAbilityBase()
{
	// 生肖大招默认配置
}

bool UDBAZodiacUltimateAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (ASC)
	{
		// 检查终极能量是否满 100
		return ASC->HasEnoughUltimateEnergy(100.0f);
	}

	return false;
}

bool UDBAZodiacUltimateAbilityBase::CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	if (!Super::CommitAbilityCost(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	// 仅服务端权威扣除 UltimateEnergy
	UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (ASC && ActorInfo->IsNetAuthority())
	{
		ASC->ConsumeUltimateEnergy(100.0f);
	}

	return true;
}
