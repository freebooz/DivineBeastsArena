// Copyright Freebooz Games, Inc. All Rights Reserved.
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
	// ==================== 可被子类重写的方法 ====================

	/** 从 DataTable 获取配置数据 (可重写) */
	UFUNCTION(BlueprintNativeEvent, Category = "DBA|Config")
	void OnSkillConfigLoaded();

	/** 技能激活时调用 (可重写) */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Gameplay")
	void OnAbilityActivatedBP(FName InSkillID);

	/** 技能结束时调用 (可重写) */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Gameplay")
	void OnAbilityEndedBP(FName InSkillID, bool bWasCancelled);

public:
	// ==================== 辅助方法 ====================

	/** 获取当前技能的 SkillID */
	FName GetSkillID() const { return SkillID; }
};
