// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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
