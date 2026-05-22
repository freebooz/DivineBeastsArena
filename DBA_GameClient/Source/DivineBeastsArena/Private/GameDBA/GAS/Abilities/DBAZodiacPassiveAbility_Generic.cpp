// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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
