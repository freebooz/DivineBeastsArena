// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Abilities/DBAElementSkillAbility_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"

UDBAElementSkillAbility_Generic::UDBAElementSkillAbility_Generic()
{
}

void UDBAElementSkillAbility_Generic::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogDBACombat, Log, TEXT("UDBAElementSkillAbility_Generic::ActivateAbility - SkillID: %s"), *SkillID.ToString());

	// 调用蓝图事件
	OnAbilityActivatedBP(SkillID);

	// 调用 Native 事件 (用于 C++ 子类重写)
	OnSkillConfigLoaded();
}

void UDBAElementSkillAbility_Generic::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UE_LOG(LogDBACombat, Log, TEXT("UDBAElementSkillAbility_Generic::EndAbility - SkillID: %s, bWasCancelled: %d"), *SkillID.ToString(), bWasCancelled);

	// 调用蓝图事件
	OnAbilityEndedBP(SkillID, bWasCancelled);

	// 调用基类 EndAbility
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDBAElementSkillAbility_Generic::OnSkillConfigLoaded_Implementation()
{
	// 默认实现为空
	// 子类或蓝图可以重写此方法以加载技能配置
}
