// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Abilities/DBAZodiacUltimateAbility_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"

UDBAZodiacUltimateAbility_Generic::UDBAZodiacUltimateAbility_Generic()
{
}

void UDBAZodiacUltimateAbility_Generic::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogDBACombat, Log, TEXT("[UDBAZodiacUltimateAbility_Generic] 激活生肖终极技能：技能ID=%s"), *UltimateSkillID.ToString());

	// 调用蓝图事件
	OnUltimateActivatedBP(UltimateSkillID);

	// 调用基类 ActivateAbility
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UDBAZodiacUltimateAbility_Generic::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UE_LOG(LogDBACombat, Log, TEXT("[UDBAZodiacUltimateAbility_Generic] 结束生肖终极技能：技能ID=%s 是否取消=%s"), *UltimateSkillID.ToString(), bWasCancelled ? TEXT("是") : TEXT("否"));

	// 调用蓝图事件
	OnUltimateEndedBP(UltimateSkillID, bWasCancelled);

	// 调用基类 EndAbility
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
