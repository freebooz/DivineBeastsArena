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
	UE_LOG(LogDBACombat, Log, TEXT("[UDBAZodiacPassiveAbility_Generic] 激活生肖被动技能：技能ID=%s"), *PassiveSkillID.ToString());

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
	UE_LOG(LogDBACombat, Log, TEXT("[UDBAZodiacPassiveAbility_Generic] 结束生肖被动技能：技能ID=%s 是否取消=%s"), *PassiveSkillID.ToString(), bWasCancelled ? TEXT("是") : TEXT("否"));

	// 调用蓝图事件
	OnPassiveRemovedBP(PassiveSkillID);

	// 调用基类 EndAbility
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
