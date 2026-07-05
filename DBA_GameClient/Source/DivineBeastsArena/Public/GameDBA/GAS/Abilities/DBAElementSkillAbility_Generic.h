// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 泛化主动技能类 - 替代 Q/W/E 各 12 个共 36 个技能

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "DBAElementSkillAbility_Generic.generated.h"

class UDataTable;

/**
 * UDBAElementSkillAbility_Generic
 * 泛化主动技能类 (替代 Q/W/E 三组各 12 个共 36 个)
 * 通过 SkillID 从 DataTable 读取配置
 *
 * 使用方式:
 * 1. 在蓝图中设置 SkillID 和 AbilityTable
 * 2. 在 DataTable 中配置技能效果参数
 * 3. 运行时通过 ActivateAbility 读取配置并执行效果
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API UDBAElementSkillAbility_Generic : public UDBAElementAbilityBase
{
	GENERATED_BODY()

public:
	UDBAElementSkillAbility_Generic();

protected:
	// ==================== UGameplayAbility Overrides ====================

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

public:
	// ==================== 配置 ====================

	/** DataTable 中定义的 SkillID */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Config")
	FName SkillID;

	/** 技能 DataTable 引用 (可选，如果不为空则优先使用) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Config")
	TObjectPtr<UDataTable> AbilityTable;

protected:
	// ==================== C++ 生命周期扩展点 ====================

	/** 从 DataTable 获取配置数据，运行时逻辑必须由 C++ 子类承载。 */
	virtual void OnSkillConfigLoaded();

	/** 技能激活时调用，运行时逻辑必须由 C++ 子类承载。 */
	virtual void OnAbilityActivated(FName InSkillID);

	/** 技能结束时调用，运行时逻辑必须由 C++ 子类承载。 */
	virtual void OnAbilityEnded(FName InSkillID, bool bWasCancelled);

public:
	// ==================== 辅助方法 ====================

	/** 获取当前技能的 SkillID */
	FName GetSkillID() const { return SkillID; }
};
