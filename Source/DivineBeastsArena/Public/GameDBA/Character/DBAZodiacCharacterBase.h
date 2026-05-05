// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色模型基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameMoba/RPC/DBARpcHandler.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "GameDBA/Combat/DBACombatTypes.h"
#include "GameDBA/Spectator/DBAObserverTypes.h"
#include "DBAZodiacCharacterBase.generated.h"

class UDBAZodiacAnimInstance;
class UDBAAbilitySystemComponent;
class ADBARpcHandler;

/**
 * DBAZodiacCharacterBase
 * 生肖角色基类
 * 提供角色公共功能：动画控制、属性同步
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ADBAZodiacCharacterBase();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	// ==================== 组件获取 ====================

	/** 获取动画实例 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	UDBAZodiacAnimInstance* GetZodiacAnimInstance() const;

	/** 获取能力系统组件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	UDBAAbilitySystemComponent* GetDBAAbilitySystemComponent() const;

	/** 获取RPC处理器 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	ADBARpcHandler* GetRpcHandler() const { return RpcHandler; }

public:
	// ==================== 属性访问 ====================

	/** 获取当前生命值 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	float GetCurrentHealth() const;

	/** 获取最大生命值 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	float GetMaxHealth() const;

	/** 获取当前能量 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	float GetCurrentEnergy() const;

	/** 获取终极能量 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	float GetUltimateEnergy() const { return UltimateEnergy; }

	/** 获取连锁等级 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	int32 GetChainLevel() const { return ChainLevel; }

	/** 获取共鸣等级 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	int32 GetResonanceLevel() const { return ResonanceLevel; }

public:
	// ==================== 属性修改 ====================

	/** 设置终极能量 (服务端调用) */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	void SetUltimateEnergy(float Value);

	/** 增加终极能量 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	void AddUltimateEnergy(float Delta);

	/** 增加连锁等级 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	void AddChainLevel(int32 Delta);

	/** 重置连锁等级 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	void ResetChainLevel();

public:
	// ==================== 动画接口 ====================

	/** 播放攻击动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void PlayAttackAnimation();

	/** 播放受击动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void PlayHitAnimation();

	/** 播放死亡动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void PlayDeathAnimation();

	/** 设置移动速度 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetAnimMoveSpeed(float Speed);

protected:
	// ==================== 配置 ====================

	/** 角色生肖类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	EDBAZodiacType ZodiacType = EDBAZodiacType::None;

	/** 角色元素类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	EDBAElementType ElementType = EDBAElementType::None;

public:
	// ==================== 移动配置 ====================

	/** 最大行走速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Movement", meta = (UIMin = 100.0, UIMax = 1200.0))
	float MaxWalkSpeed = 600.0f;

	/** 最大奔跑速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Movement", meta = (UIMin = 100.0, UIMax = 1500.0))
	float MaxRunSpeed = 900.0f;

	/** 停止减速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Movement")
	float BrakingDeceleration = 2048.0f;

public:
	// ==================== RPC 相关 ====================

	/** RPC 处理器类 */
	UPROPERTY(EditDefaultsOnly, Category = "RPC")
	TSubclassOf<ADBARpcHandler> RpcHandlerClass;

	/** RPC 处理器实例 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPC")
	ADBARpcHandler* RpcHandler;

public:
	// ==================== 死亡状态 ====================

	/** 获取死亡状态 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Death")
	EDADeathState GetDeathState() const { return DeathState; }

	/** 是否已死亡 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Death")
	bool IsDead() const { return DeathState == EDADeathState::Dead || DeathState == EDADeathState::Dying; }

	/** 触发死亡 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Death")
	void OnDeath();

	/** 复活 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Death")
	void OnRevive();

public:
	// ==================== 复制属性 ====================

	/** 终极能量 (用于终极技能) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Combat")
	float UltimateEnergy = 0.0f;

	/** 当前连锁等级 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Combat")
	int32 ChainLevel = 0;

	/** 当前共鸣等级 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Combat")
	int32 ResonanceLevel = 0;

	/** 死亡状态 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Death")
	EDADeathState DeathState = EDADeathState::Alive;

public:
	// ==================== 队伍信息 ====================

	/** 获取队伍ID */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Team")
	int32 GetTeamID() const { return TeamID; }

	/** 设置队伍ID */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Team")
	void SetTeamID(int32 NewTeamID);

	/** 是否是队友 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Team")
	bool IsTeammate(const ADBAZodiacCharacterBase* Other) const;

public:
	/** 队伍ID (复制) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Team")
	int32 TeamID;

	/** 英雄ID (用于观战显示) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Config")
	FName HeroID;

public:
	// ==================== 观战数据接口 ====================

	/** 获取观战数据 (用于观战系统) */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Spectator")
	void GetSpectatorData(FDBAObserverViewTarget& OutData) const;

	/** 获取技能冷却数组 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Spectator")
	TArray<float> GetSkillCooldowns() const { return SkillCooldowns; }

	/** 获取技能最大冷却数组 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Spectator")
	TArray<float> GetSkillMaxCooldowns() const { return SkillMaxCooldowns; }

	/** 是否终极技能就绪 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Spectator")
	bool IsUltimateReady() const { return UltimateEnergy >= 100.0f; }

public:
	/** 技能冷却数组 (观战用) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Spectator")
	TArray<float> SkillCooldowns;

	/** 技能最大冷却数组 (观战用) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Spectator")
	TArray<float> SkillMaxCooldowns;

public:
	/** 更新技能冷却 (服务端调用) */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Spectator")
	void UpdateSkillCooldowns(const TArray<float>& NewCooldowns);
};