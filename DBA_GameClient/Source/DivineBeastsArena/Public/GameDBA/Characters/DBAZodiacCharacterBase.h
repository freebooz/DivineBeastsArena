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
#include "GameDBA/Characters/DBACharacterBase.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "GameMoba/Targeting/DBACombatTypes.h"
#include "GameDBA/Gameplay/Loadout/DBAPlayableSkillTypes.h"
#include "GameDBA/Spectator/DBAObserverTypes.h"
#include "GameDBA/Characters/IDBACharacterRef.h"
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
 * 继承 ADBACharacterBase（统一角色基类，提供 DeathState/TeamID/IAbilitySystemInterface），
 * 并实现 IDBACharacterRef 接口，让 RPC Handler 可以通过接口访问角色属性。
 * ASC 来自 PlayerState（玩家角色标准做法），通过 GetAbilitySystemComponent() 重写暴露。
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacterBase : public ADBACharacterBase, public IIDBACharacterRef
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

	/** 由大厅服务器在生成 Pawn 后设置；客户端通过复制回调刷新统一模型的生肖外观。 */
	void SetLobbyDisplayZodiac(EDBAZodiac NewZodiac);

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

	/** 获取当前生命值（重写基类与 IDBACharacterRef，从 GAS AttributeSet 读取） */
	virtual float GetCurrentHealth() const override;

	/** 获取最大生命值（重写基类与 IDBACharacterRef，从 GAS AttributeSet 读取） */
	virtual float GetMaxHealth() const override;

	/** 获取当前能量（IDBACharacterRef） */
	virtual float GetCurrentEnergy() const override;

	/** 获取英雄等级 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Attribute")
	int32 GetHeroLevel() const;

	/** 获取终极能量（IDBACharacterRef） */
	virtual float GetUltimateEnergy() const override;

	/** 获取连锁等级（IDBACharacterRef） */
	virtual int32 GetChainLevel() const override;

	/** 获取共鸣等级（IDBACharacterRef） */
	virtual int32 GetResonanceLevel() const override;

public:
	// ==================== IDBACharacterRef 接口实现 ====================

	/** 获取最大能量 */
	virtual float GetMaxEnergy() const override;

	/** 获取 AbilitySystemComponent */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 获取元素类型 */
	virtual EDBAElementType GetElementType() const override;

	/** 是否已死亡（IDBACharacterRef，转发基类实现以消解接口与基类同名函数的二义性） */
	virtual bool IsDead() const override;

	/** 获取队伍 ID（IDBACharacterRef，转发基类实现以消解接口与基类同名函数的二义性） */
	virtual int32 GetTeamID() const override;

	/** 检查技能是否在冷却中 */
	virtual bool IsAbilityOnCooldown(FName SkillId) const override;

	/** 检查是否有足够的能量 */
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_ZodiacType, Category = "DBA|Config")
	EDBAZodiacType ZodiacType = EDBAZodiacType::None;

	UFUNCTION()
	void OnRep_ZodiacType();

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
	bool bLobbyVisualLoadRequested = false;

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

	// GetDeathState() 和 IsDead() 由基类 ADBACharacterBase 提供，此处不再重复声明
	// DeathState 字段由基类 ADBACharacterBase 持有（Replicated）

	/** 触发死亡 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Death")
	void OnDeath();

	/** 复活 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Death")
	void OnRevive();

public:
	// ==================== 复制属性 ====================

	// P0-3 修复：已移除 UltimateEnergy / ChainLevel / ResonanceLevel 三个 Replicated 死字段
	// 原因：写入路径已委托 ASC，字段永远不会被写入，浪费带宽并误导开发者直接读写绕过 ASC
	// 读取请使用 GetUltimateEnergy() / GetChainLevel() / GetResonanceLevel()（优先从 ASC 读取）
	// DeathState 和 TeamID 字段由基类 ADBACharacterBase 持有，此处不再重复声明

public:
	// ==================== 队伍信息 ====================

	// GetTeamID() 由基类 ADBACharacterBase 提供，此处不再重复声明
	// TeamID 字段由基类 ADBACharacterBase 持有（Replicated）

	/** 设置队伍ID（重写基类，增加非负约束） */
	virtual void SetTeamID(int32 NewTeamID) override;

	/** 是否是队友 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character|Team")
	bool IsTeammate(const ADBAZodiacCharacterBase* Other) const;

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
