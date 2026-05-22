// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 泛化被动技能类 - 替代 Passive 技能 12 个

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/GAS/Abilities/DBAZodiacAbilityBase.h"
#include "DBAZodiacPassiveAbility_Generic.generated.h"

class UDataTable;

/**
 * UDBAZodiacPassiveAbility_Generic
 * 泛化被动技能类 (替代 Passive 技能 12 个)
 * 通过 SkillID 从 DataTable 读取配置
 * 被动技能自动生效，不占用输入
 *
 * 使用方式:
 * 1. 在蓝图中设置 SkillID 和 PassiveTable
 * 2. 在 DataTable 中配置被动效果参数
 * 3. 运行时通过 ActivateAbility 读取配置并应用被动效果
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API UDBAZodiacPassiveAbility_Generic : public UDBAZodiacAbilityBase
{
	GENERATED_BODY()

public:
	UDBAZodiacPassiveAbility_Generic();

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

	/** DataTable 中定义的 PassiveSkillID */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Config")
	FName PassiveSkillID;

	/** 被动技能 DataTable 引用 (可选) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Config")
	TObjectPtr<UDataTable> PassiveTable;

protected:
	// ==================== 可被子类重写的方法 ====================

	/** 被动激活时调用 (可重写) */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Gameplay")
	void OnPassiveActivatedBP(FName InSkillID);

	/** 被动移除时调用 (可重写) */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Gameplay")
	void OnPassiveRemovedBP(FName InSkillID);

public:
	// ==================== 辅助方法 ====================

	/** 获取当前被动技能的 SkillID */
	FName GetPassiveSkillID() const { return PassiveSkillID; }
};
