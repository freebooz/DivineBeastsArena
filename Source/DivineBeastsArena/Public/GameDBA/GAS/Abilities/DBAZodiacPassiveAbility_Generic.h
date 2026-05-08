// Copyright Freebooz Games, Inc. All Rights Reserved.
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
