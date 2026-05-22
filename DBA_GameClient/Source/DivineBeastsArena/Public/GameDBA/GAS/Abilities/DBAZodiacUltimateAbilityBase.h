// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameDBA/GAS/Abilities/DBAZodiacAbilityBase.h"
#include "DBAZodiacUltimateAbilityBase.generated.h"

/**
 * 生肖大招基类 (ZodiacUltimate)
 * 必须消耗 100 点 UltimateEnergy
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAZodiacUltimateAbilityBase : public UDBAZodiacAbilityBase
{
	GENERATED_BODY()

public:
	UDBAZodiacUltimateAbilityBase();

protected:
	/**
	 * 检查 UltimateEnergy 是否达到 100
	 * 服务端与客户端预测均执行
	 */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/**
	 * 消耗 100 点 UltimateEnergy
	 */
	virtual bool CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
};
