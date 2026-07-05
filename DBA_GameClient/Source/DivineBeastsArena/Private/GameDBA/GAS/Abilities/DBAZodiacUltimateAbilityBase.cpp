// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 生肖终极能力基类实现 - 生肖大招能力的基类

#include "GameDBA/GAS/Abilities/DBAZodiacUltimateAbilityBase.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"

// 构造函数 - 初始化生肖终极能力
UDBAZodiacUltimateAbilityBase::UDBAZodiacUltimateAbilityBase()
{
	// 生肖大招默认配置
}

// CanActivateAbility - 检查能力是否可以激活
bool UDBAZodiacUltimateAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 调用父类检查
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 获取能力系统组件
	UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (ASC)
	{
		// 检查终极能量是否已达到当前大招阈值
		return ASC->HasEnoughUltimateEnergy(DBAConstants::MaxUltimateEnergy);
	}

	return false;
}

// CommitAbilityCost - 提交能力消耗（终极能量）
bool UDBAZodiacUltimateAbilityBase::CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	// 调用父类提交消耗
	if (!Super::CommitAbilityCost(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	// 仅服务端权威扣除 UltimateEnergy
	UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (ASC && ActorInfo->IsNetAuthority())
	{
		// 消耗当前配置的大招能量阈值
		ASC->ConsumeUltimateEnergy(DBAConstants::MaxUltimateEnergy);
	}

	return true;
}
