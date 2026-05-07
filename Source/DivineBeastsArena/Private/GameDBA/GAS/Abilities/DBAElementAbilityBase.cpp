// Copyright FreeboozStudio. All Rights Reserved.

#include "GameDBA/GAS/Abilities/DBAElementAbilityBase.h"

#include "AbilitySystemComponent.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"

UDBAElementAbilityBase::UDBAElementAbilityBase()
{
	AbilityElementType = EDBAElement::None;
	AbilityEnergyCost = 0.0f;
	EnergyCost = 0.0f;
}

bool UDBAElementAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const UDBABattleAttributeSet* AttrSet = ASC ? ASC->GetSet<UDBABattleAttributeSet>() : nullptr;
	return AttrSet && AttrSet->GetCurrentEnergy() >= FMath::Max(AbilityEnergyCost, EnergyCost);
}

bool UDBAElementAbilityBase::CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags)
{
	return Super::CommitAbilityCost(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
}
