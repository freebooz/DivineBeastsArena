// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Gameplay/Abilities/DBAElementSkillAbility_Generic.h"
#include "GameplayTagContainer.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Gameplay/GAS/DBAAbilitySystemComponent.h"

UDBAElementSkillAbility_Generic::UDBAElementSkillAbility_Generic()
{
}

void UDBAElementSkillAbility_Generic::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[UDBAElementSkillAbility_Generic] 激活元素技能失败：提交消耗或冷却失败，技能ID=%s"), *SkillID.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogDBACombat, Log, TEXT("[UDBAElementSkillAbility_Generic] 激活元素技能：技能ID=%s"), *SkillID.ToString());

	if (ActorInfo)
	{
		if (UDBAAbilitySystemComponent* DBAASC = Cast<UDBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
		{
			const FGameplayTag CastCueTag = FGameplayTag::RequestGameplayTag(FName(TEXT("GameplayCue.DBA.Skill.Cast")), false);
			DBAASC->TriggerGameplayCue(CastCueTag, ActorInfo->AvatarActor.Get());
		}
	}

	OnAbilityActivated(SkillID);

	OnSkillConfigLoaded();
}

void UDBAElementSkillAbility_Generic::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UE_LOG(LogDBACombat, Log, TEXT("[UDBAElementSkillAbility_Generic] 结束元素技能：技能ID=%s 是否取消=%s"), *SkillID.ToString(), bWasCancelled ? TEXT("是") : TEXT("否"));

	OnAbilityEnded(SkillID, bWasCancelled);

	// 调用基类 EndAbility
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDBAElementSkillAbility_Generic::OnSkillConfigLoaded()
{
}

void UDBAElementSkillAbility_Generic::OnAbilityActivated(FName InSkillID)
{
}

void UDBAElementSkillAbility_Generic::OnAbilityEnded(FName InSkillID, bool bWasCancelled)
{
}
