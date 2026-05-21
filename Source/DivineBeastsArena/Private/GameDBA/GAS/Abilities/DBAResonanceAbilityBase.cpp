// Copyright FreeboozStudio. All Rights Reserved.

#include "GameDBA/GAS/Abilities/DBAResonanceAbilityBase.h"

#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"

UDBAResonanceAbilityBase::UDBAResonanceAbilityBase()
{
}

void UDBAResonanceAbilityBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const UDBAAbilitySystemComponent* ASC = ActorInfo ? Cast<UDBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()) : nullptr;
	if (ASC && ActorInfo->IsNetAuthority())
	{
		ApplyResonanceEffect(ASC->GetResonanceLevel());
	}
}

void UDBAResonanceAbilityBase::ApplyResonanceEffect(int32 CurrentResonanceLevel)
{
	UE_LOG(LogDBACombat, Verbose, TEXT("[DBAResonanceAbilityBase] 当前共鸣等级：%d"), CurrentResonanceLevel);
}
