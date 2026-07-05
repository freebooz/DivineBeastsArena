// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 生肖角色模型基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "GameDBA/Combat/DBACombatTypes.h"
#include "GameDBA/Combat/DBAPlayableSkillTypes.h"
#include "GameDBA/Spectator/DBAObserverTypes.h"
#include "GameDBA/Character/IDBACharacterRef.h"
#include "DBAZodiacCharacterBase.generated.h"

class UDBAZodiacAnimInstance;
class UDBAAbilitySystemComponent;
class UAnimationAsset;
class UCameraComponent;
class USpringArmComponent;
class ADBARpcHandler;
class UDBAPlayableSkillComponent;
struct FDBAPlayableSkillRuntimeSpec;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillCooldownsChanged, const TArray<float>&, Cooldowns);

/**
 * DBAZodiacCharacterBase
 * 生肖角色基类
 * 提供角色公共功能：动画控制、属性同步
 *
 * 实现了 IDBACharacterRef 接口，让 RPC Handler 可以通过接口访问角色属性
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacterBase : public ACharacter, public IIDBACharacterRef
{
	GENERATED_BODY()

public:
	ADBAZodiacCharacterBase();

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void InitializeDBAAbilityActorInfo();
	void ApplyLobbyVisuals();

public:
	// ==================== 组件获取 ====================

	/** 获取动画实例 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	UDBAZodiacAnimInstance* GetZodiacAnimInstance() const;

	/** 获取能力系统组件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	UDBAAbilitySystemComponent* GetDBAAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Skill")
	UDBAPlayableSkillComponent* GetPlayableSkillComponent() const { return PlayableSkillComponent; }

	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Skill")
	TArray<FDBAPlayableSkillRuntimeSpec> GetPlayableSkillSpecs() const;

	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	EDBAZodiacType GetZodiacType() const { return ZodiacType; }

	/** 获取RPC处理器 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Camera")
	USpringArmComponent* GetLobbyCameraBoom() const { return LobbyCameraBoom; }

	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Camera")
	UCameraComponent* GetLobbyFollowCamera() const { return LobbyFollowCamera; }

	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	ADBARpcHandler* GetRpcHandler() const { return RpcHandler; }

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Spell")
	void CastLobbyFireball();

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Spell")
	void CastLobbyFireballAtTarget(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Spell")
	void CastEquippedSkill(int32 SkillSlot);

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Spell")
	void CastEquippedSkillAtTarget(int32 SkillSlot, AActor* TargetActor);

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

	/** 获取英雄等级 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	int32 GetHeroLevel() const;

	/** 获取终极能量 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	float GetUltimateEnergy() const;

	/** 获取连锁等级 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	int32 GetChainLevel() const;

	/** 获取共鸣等级 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	int32 GetResonanceLevel() const;

public:
	// ==================== IDBACharacterRef 接口实现 ====================

	/** 获取最大能量 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	virtual float GetMaxEnergy() const override;

	/** 获取 AbilitySystemComponent */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 获取元素类型 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	virtual EDBAElementType GetElementType() const override;

	/** 检查技能是否在冷却中 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	virtual bool IsAbilityOnCooldown(FName SkillId) const override;

	/** 检查是否有足够的能量 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	virtual bool HasEnoughEnergy(float Cost) const override;

	/** 标记实现 IIDBACharacterRef 接口 */
	virtual bool ImplementsIIDBACharacterRef() const { return true; }

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
	void SyncArenaHUDFromAttributes(bool bForce = false);
	void BindArenaHUDAttributeDelegates();
	void UnbindArenaHUDAttributeDelegates();
	void HandleArenaHUDAttributeChanged(const FOnAttributeChangeData& ChangeData);

	UFUNCTION()
	void HandleArenaHUDUltimateEnergyChanged(float CurrentEnergy, float MaxEnergy);

	UFUNCTION()
	void HandleArenaHUDChainLevelChanged(int32 ChainLevel);

	UFUNCTION()
	void HandleArenaHUDResonanceLevelChanged(int32 ResonanceLevel);

	UFUNCTION()
	void HandleArenaHUDSkillCueExecuted(FName SkillId, AActor* Target);

	FText ResolveArenaHUDSkillCueDisplayName(FName SkillId) const;

protected:
	// ==================== 配置 ====================

	/** 角色生肖类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	EDBAZodiacType ZodiacType = EDBAZodiacType::None;

	/** 角色元素类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	EDBAElementType ElementType = EDBAElementType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ArenaHUDCriticalHealthRatioThreshold = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ArenaHUDCriticalEnergyRatioThreshold = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (ClampMin = "0.0"))
	float ArenaHUDChainReadyAnnouncementDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (ClampMin = "0.0"))
	float ArenaHUDSkillCueAnnouncementDuration = 1.5f;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCastLobbyFireball(FVector_NetQuantizeNormal AimDirection);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCastLobbyFireballAtTarget(AActor* TargetActor, FVector_NetQuantizeNormal FallbackAimDirection);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCastEquippedSkill(int32 SkillSlot, AActor* TargetActor, FVector_NetQuantizeNormal FallbackAimDirection);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayLobbySkillCastFeedback(int32 SkillSlot);

	void CastLobbyFireballInternal(const FVector& AimDirection, AActor* TargetActor = nullptr);
	void CastEquippedSkillInternal(int32 SkillSlot, const FVector& AimDirection, AActor* TargetActor = nullptr);
	bool ValidateServerEquippedSkillCast(int32 SkillSlot, AActor* TargetActor) const;
	void PlayLobbySkillCastFeedbackLocal(int32 SkillSlot);
	void UpdateLobbyLocomotionAnimation();
	UAnimationAsset* LoadLobbyAnimation(const FString& AnimationPath);

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Lobby|Animation")
	float LobbyRunAnimationThreshold = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Lobby|Animation")
	float LobbyAttackAnimationDuration = 0.9f;

public:
	// ==================== RPC 相关 ====================

	/** RPC 处理器类 */
	UPROPERTY(EditDefaultsOnly, Category = "RPC")
	TSubclassOf<ADBARpcHandler> RpcHandlerClass;

	/** RPC 处理器实例 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPC")
	ADBARpcHandler* RpcHandler;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Camera")
	TObjectPtr<USpringArmComponent> LobbyCameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Camera")
	TObjectPtr<UCameraComponent> LobbyFollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Skill")
	TObjectPtr<UDBAPlayableSkillComponent> PlayableSkillComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> LobbyIdleAnimation;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> LobbyRunAnimation;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> LobbyAttackAnimation;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CurrentLobbyAnimation;

	float LobbyAttackAnimationTimeRemaining = 0.0f;
	bool bUseLobbySingleNodeLocomotion = false;

	bool bHasSyncedArenaHUDAttributes = false;
	bool bHasBoundArenaHUDAttributeDelegates = false;
	TWeakObjectPtr<UDBAAbilitySystemComponent> ArenaHUDAttributeDelegateASC;
	FDelegateHandle ArenaHUDCurrentHealthChangedHandle;
	FDelegateHandle ArenaHUDMaxHealthChangedHandle;
	FDelegateHandle ArenaHUDCurrentEnergyChangedHandle;
	FDelegateHandle ArenaHUDMaxEnergyChangedHandle;
	FDelegateHandle ArenaHUDHeroLevelChangedHandle;
	float LastSyncedArenaHUDCurrentHP = 0.0f;
	float LastSyncedArenaHUDMaxHP = 0.0f;
	float LastSyncedArenaHUDCurrentEnergy = 0.0f;
	float LastSyncedArenaHUDMaxEnergy = 0.0f;
	float LastSyncedArenaHUDUltimateEnergy = 0.0f;
	float LastSyncedArenaHUDMaxUltimateEnergy = DBAConstants::MaxUltimateEnergy;
	int32 LastSyncedArenaHUDHeroLevel = 1;
	int32 LastSyncedArenaHUDChainLevel = 0;
	int32 LastSyncedArenaHUDResonanceLevel = 0;
	bool bHasSyncedArenaHUDCriticalState = false;
	bool LastSyncedArenaHUDBLowHP = false;
	bool LastSyncedArenaHUDBLowEnergy = false;
	bool bLastSyncedArenaHUDChainReady = false;
	bool bHasSyncedArenaHUDUltimateReadyPrompt = false;
	bool bLastSyncedArenaHUDUltimateReady = false;

	FTimerHandle DeathStateFinalizeTimerHandle;

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
	bool IsUltimateReady() const;

public:
	/** 技能冷却数组 (观战用) */
	UPROPERTY(ReplicatedUsing = OnRep_SkillCooldowns, BlueprintReadOnly, Category = "DBA|Spectator")
	TArray<float> SkillCooldowns;

	/** 技能最大冷却数组 (观战用) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "DBA|Spectator")
	TArray<float> SkillMaxCooldowns;

	UPROPERTY(BlueprintAssignable, Category = "DBA|Character|Spectator")
	FOnSkillCooldownsChanged OnSkillCooldownsChanged;

public:
	/** 更新技能冷却 (服务端调用) */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Spectator")
	void UpdateSkillCooldowns(const TArray<float>& NewCooldowns);

protected:
	UFUNCTION()
	void OnRep_SkillCooldowns();
};
