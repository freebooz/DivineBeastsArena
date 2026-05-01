// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "RPC/DBARpcInterface.h"
#include "DBAMobaGameplayAbilityBase.generated.h"

class UDBAMobaAbilitySystemComponentBase;

/**
 * Ability 激活策略
 * 定义 Ability 在何种条件下可以激活
 */
UENUM(BlueprintType)
enum class EDBAMobaAbilityActivationPolicy : uint8
{
	/** 收到输入时激活 */
	OnInputTriggered UMETA(DisplayName = "输入触发"),

	/** Avatar 生成时自动激活 */
	OnSpawn UMETA(DisplayName = "生成时激活"),

	/** 被动技能，始终激活 */
	Passive UMETA(DisplayName = "被动技能")
};

/**
 * MOBA 游戏 GameplayAbility Base 层
 *
 * 职责：
 * - 定义 Ability 激活校验基类
 * - 提供 InputTag / AbilityTag / CooldownTag / BlockTag 基本链路
 * - 处理 Authority / Prediction / Replication 边界
 * - 提供客户端本地反馈与服务端裁定接口
 * - 支持 Dedicated Server 不创建表现资源但保留 Cue / 事件桥接接口
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBAMobaGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UDBAMobaGameplayAbilityBase();

	// ========== Ability 配置 ==========

	/** Ability 激活策略 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (DisplayName = "激活策略"))
	EDBAMobaAbilityActivationPolicy ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	/** 是否在服务端执行（Authority） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (DisplayName = "服务端执行"))
	bool bServerAuthority = true;

	/** 是否支持客户端预测 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (DisplayName = "客户端预测"))
	bool bClientPrediction = false;

	/** 输入 Tag（用于绑定输入） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Tags", meta = (DisplayName = "输入 Tag"))
	FGameplayTag InputTag;

	/** Ability Tag（用于标识 Ability） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Tags", meta = (DisplayName = "Ability Tag"))
	FGameplayTag AbilityTag;

	/** 冷却 Tag（用于冷却检测） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Tags", meta = (DisplayName = "冷却 Tag"))
	FGameplayTag CooldownTag;

	/** 阻止 Tag（拥有这些 Tag 时无法激活） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Tags", meta = (DisplayName = "阻止 Tag"))
	FGameplayTagContainer BlockTags;

	// ========== GameplayAbility 接口实现 ==========

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// ========== 客户端本地反馈接口 ==========

	/**
	 * 客户端本地反馈（预测）
	 * 用于播放动画预览、音效、粒子等表现
	 * Dedicated Server 不调用此函数
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability|Client Feedback")
	void OnClientLocalFeedback();
	virtual void OnClientLocalFeedback_Implementation();

	/**
	 * 服务端裁定完成回调
	 * 用于客户端接收服务端裁定结果后的处理
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability|Server Authority")
	void OnServerAuthorityConfirmed();
	virtual void OnServerAuthorityConfirmed_Implementation();

	/**
	 * 服务端裁定失败回调
	 * 用于客户端预测失败后的回滚处理
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability|Server Authority")
	void OnServerAuthorityRejected();
	virtual void OnServerAuthorityRejected_Implementation();

	// ========== GameplayCue 桥接接口 ==========

	/**
	 * 触发 GameplayCue（支持 Dedicated Server）
	 * Dedicated Server 不创建表现资源，但保留事件桥接接口
	 * @param CueTag GameplayCue Tag
	 * @param Parameters GameplayCue 参数
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|GameplayCue")
	void ExecuteGameplayCueOnTarget(FGameplayTag CueTag, const FGameplayCueParameters& Parameters);

	// ========== 辅助函数 ==========

	/**
	 * 获取 ASC
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	UDBAMobaAbilitySystemComponentBase* GetDBAAbilitySystemComponent() const;

	/**
	 * 检查是否在冷却中
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	bool IsOnCooldown() const;

	/**
	 * 获取剩余冷却时间
	 */
	float GetCooldownTimeRemaining(const FGameplayAbilityActorInfo* ActorInfo) const override;

	// ========================================
	// 伤害计算辅助函数
	// ========================================

	/**
	 * 计算考虑防御的最终伤害
	 * DamageReduction = Defense / (Defense + 100)
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Combat")
	float CalculateDamageWithDefense(float BaseDamage, float TargetDefense) const;

	/**
	 * 应用暴击判定
	 * 返回暴击后的伤害值
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Combat")
	float ApplyCriticalHit(float Damage, float CriticalRate, float CriticalMultiplier) const;

	/**
	 * 对目标应用伤害
	 * 服务端权威调用
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Combat")
	void ApplyDamageToTarget(AActor* TargetActor, float BaseDamage, const FGameplayTagContainer& DamageTags);

	/**
	 * 对目标触发 GameplayCue
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Cue")
	void TriggerGameplayCueOnTarget(FGameplayTag CueTag, AActor* TargetActor, float Magnitude = 0.0f);

	/**
	 * 从 EventData 获取目标 Actor（const 版本）
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Helper")
	const AActor* GetTargetActorFromEventData(const FGameplayEventData& EventData) const;

	// ========== RPC 接口 ==========

	/**
	 * 调用服务端尝试激活技能
	 * 客户端通过此函数向服务端发送技能激活请求
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|RPC")
	void CallServerTryActivateAbility(AActor* TargetActor, FVector TargetLocation);

	/**
	 * 获取 RPC 处理器
	 * 用于获取角色身上的 RPC Handler 引用
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|RPC")
	ADBARpcHandler* GetRpcHandler() const;

protected:
	/** 伤害相关 Tag 容器 */
	UPROPERTY(BlueprintReadOnly, Category = "Ability|Tags")
	FGameplayTagContainer DamageTags;

	/**
	 * 服务端权威激活逻辑
	 * 子类重写此函数实现具体技能逻辑
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnServerActivate();
	virtual void OnServerActivate_Implementation();
};
