// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MobaBase/GAS/DBAMobaAbilitySystemComponentBase.h"
#include "Common/DBAEnumsCore.h"
#include "Common/DBAConstants.h"
#include "DBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "DBAAbilitySystemComponent.generated.h"

class UDBAAbilitySetDataAsset;
class UDBAMobaGameplayAbilityBase;

/**
 * 《神兽竞技场》项目层 AbilitySystemComponent
 *
 * 职责：
 * - 管理 Zodiac / Element / FiveCamp / FixedSkillGroup 授予的 Ability
 * - 管理 UltimateEnergy / ChainLevel / ResonanceLevel
 * - 服务端权威校验技能激活合法性、目标合法性、TeamId 敌我关系
 * - 桥接 GameplayCue / UI / 音效 / 动画
 * - PC / Android / DS 差异处理
 * - ZodiacUltimate 消耗 UltimateEnergy
 * - Skill01~04 消耗 CurrentEnergy 或冷却
 * - Passive / Resonance 自动授予，不绑定输入
 */
UCLASS(ClassGroup=(DBA), meta=(BlueprintSpawnableComponent))
class DIVINEBEASTSARENA_API UDBAAbilitySystemComponent : public UDBAMobaAbilitySystemComponentBase
{
	GENERATED_BODY()

public:
	UDBAAbilitySystemComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End UActorComponent Interface

	//~ Begin UAbilitySystemComponent Interface
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	//~ End UAbilitySystemComponent Interface

	// ========================================
	// 授予 Ability 接口
	// ========================================

	/**
	 * 根据 FixedSkillGroup 授予完整技能组
	 * 包含：Passive / Skill01~04 / ZodiacUltimate / Resonance
	 * 服务端权威调用
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability")
	void GrantAbilitiesFromFixedSkillGroup(const FName& FixedSkillGroupId);

	/**
	 * 移除所有已授予的 Ability
	 * 用于重新选择英雄或对局结束
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability")
	void RemoveAllGrantedAbilities();

	// ========================================
	// UltimateEnergy 管理
	// ========================================

	/**
	 * 获取当前 UltimateEnergy
	 * 范围 0~100
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ultimate")
	float GetUltimateEnergy() const { return UltimateEnergy; }

	/**
	 * 增加 UltimateEnergy
	 * 服务端权威调用
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ultimate")
	void AddUltimateEnergy(float Amount);

	/**
	 * 消耗 UltimateEnergy
	 * 服务端权威调用
	 * 返回是否成功消耗
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ultimate")
	bool ConsumeUltimateEnergy(float Amount);

	/**
	 * 检查 UltimateEnergy 是否足够
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ultimate")
	bool HasEnoughUltimateEnergy(float Amount) const;

	// ========================================
	// ChainLevel 管理
	// ========================================

	/**
	 * 获取当前 ChainLevel
	 * 范围 0~10
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Chain")
	int32 GetChainLevel() const { return ChainLevel; }

	/**
	 * 增加 ChainLevel
	 * 服务端权威调用
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Chain")
	void AddChainLevel(int32 Amount);

	/**
	 * 重置 ChainLevel
	 * 6 秒未命中敌人或连锁终结时调用
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Chain")
	void ResetChainLevel();

	/**
	 * 检查是否触发连锁终结
	 * Chain >= 10 且下一次有效命中
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Chain")
	bool ShouldTriggerChainFinisher() const;

	// ========================================
	// ResonanceLevel 管理
	// ========================================

	/**
	 * 获取当前 ResonanceLevel
	 * 范围 0~4
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Resonance")
	int32 GetResonanceLevel() const { return ResonanceLevel; }

	/**
	 * 设置 ResonanceLevel
	 * 根据同元素技能数量自动计算
	 * 服务端权威调用
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Resonance")
	void SetResonanceLevel(int32 Level);

	// ========================================
	// 技能激活合法性校验
	// ========================================

	/**
	 * 服务端校验技能是否可以激活
	 * 检查：
	 * - 技能是否存在
	 * - 是否在冷却中
	 * - CurrentEnergy 是否足够
	 * - UltimateEnergy 是否足够（ZodiacUltimate）
	 * - 目标是否合法
	 * - TeamId 敌我关系
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability")
	bool CanActivateAbility(TSubclassOf<UDBAMobaGameplayAbilityBase> AbilityClass, AActor* Target) const;

	/**
	 * 服务端校验目标是否合法
	 * 检查：
	 * - 目标是否存在
	 * - 目标是否在有效范围内
	 * - TeamId 敌我关系
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability")
	bool IsValidTarget(AActor* Target, bool bRequireEnemy) const;

	// ========================================
	// GameplayCue / UI / 音效 / 动画桥接
	// ========================================

	/**
	 * 触发 GameplayCue
	 * 用于技能表现、音效、动画
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Cue")
	void TriggerGameplayCue(const FGameplayTag& CueTag, AActor* Target);

	// ========================================
	// 网络复制
	// ========================================

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/**
	 * 当前 UltimateEnergy
	 * 范围 0~100
	 * 服务端权威，复制到客户端
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Ultimate")
	float UltimateEnergy;

	/**
	 * 当前 ChainLevel
	 * 范围 0~10
	 * 服务端权威，复制到客户端
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Chain")
	int32 ChainLevel;

	/**
	 * 当前 ResonanceLevel
	 * 范围 0~4
	 * 服务端权威，复制到客户端
	 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Resonance")
	int32 ResonanceLevel;

	/**
	 * 已授予的 Ability Handle 列表
	 * 用于移除 Ability
	 */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

	/**
	 * 上次命中敌人的时间
	 * 用于 ChainLevel 6 秒未命中归零
	 */
	UPROPERTY()
	float LastHitTime;

	/**
	 * ChainLevel 归零计时器
	 */
	FTimerHandle ChainResetTimerHandle;

	/**
	 * UltimateEnergy 被动回复计时器
	 * +1/秒
	 */
	FTimerHandle UltimateEnergyRegenTimerHandle;

	/**
	 * 被动回复 UltimateEnergy
	 * 服务端权威调用
	 */
	void PassiveRegenUltimateEnergy();

	/**
	 * 检查 ChainLevel 是否需要归零
	 * 6 秒未命中敌人
	 */
	void CheckChainReset();
};
