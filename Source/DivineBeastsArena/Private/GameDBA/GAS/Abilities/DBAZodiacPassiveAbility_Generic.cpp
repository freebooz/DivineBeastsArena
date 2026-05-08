// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Abilities/DBAZodiacPassiveAbility_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"

UDBAZodiacPassiveAbility_Generic::UDBAZodiacPassiveAbility_Generic()
{
}

void UDBAZodiacPassiveAbility_Generic::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogDBAAbility, Log, TEXT("UDBAZodiacPassiveAbility_Generic::ActivateAbility - PassiveSkillID: %s"), *PassiveSkillID.ToString());

	// 调用蓝图事件
	OnPassiveActivatedBP(PassiveSkillID);

	// 调用基类 ActivateAbility
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UDBAZodiacPassiveAbility_Generic::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UE_LOG(LogDBAAbility, Log, TEXT("UDBAZodiacPassiveAbility_Generic::EndAbility - PassiveSkillID: %s, bWasCancelled: %d"), *PassiveSkillID.ToString(), bWasCancelled);

	// 调用蓝图事件
	OnPassiveRemovedBP(PassiveSkillID);

	// 调用基类 EndAbility
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
