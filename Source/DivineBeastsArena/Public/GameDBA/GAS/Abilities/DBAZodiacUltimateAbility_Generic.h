// Copyright Freebooz Games, Inc. All Rights Reserved.
// 泛化终极技能类 - 替代 R 技能 12 个

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/GAS/Abilities/DBAZodiacUltimateAbilityBase.h"
#include "DBAZodiacUltimateAbility_Generic.generated.h"

class UDataTable;

/**
 * UDBAZodiacUltimateAbility_Generic
 * 泛化终极技能类 (替代 R 技能 12 个)
 * 通过 SkillID 从 DataTable 读取配置
 * 必须消耗 100 点 UltimateEnergy
 *
 * 使用方式:
 * 1. 在蓝图中设置 SkillID 和 UltimateTable
 * 2. 在 DataTable 中配置大招效果参数
 * 3. 运行时通过 ActivateAbility 读取配置并执行效果
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API UDBAZodiacUltimateAbility_Generic : public UDBAZodiacUltimateAbilityBase
{
	GENERATED_BODY()

public:
	UDBAZodiacUltimateAbility_Generic();

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

	/** DataTable 中定义的 UltimateSkillID */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Config")
	FName UltimateSkillID;

	/** 终极技能 DataTable 引用 (可选) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Config")
	TObjectPtr<UDataTable> UltimateTable;

protected:
	// ==================== 可被子类重写的方法 ====================

	/** 技能激活时调用 (可重写) */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Gameplay")
	void OnUltimateActivatedBP(FName InSkillID);

	/** 技能结束时调用 (可重写) */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Gameplay")
	void OnUltimateEndedBP(FName InSkillID, bool bWasCancelled);

public:
	// ==================== 辅助方法 ====================

	/** 获取当前终极技能的 SkillID */
	FName GetUltimateSkillID() const { return UltimateSkillID; }
};
