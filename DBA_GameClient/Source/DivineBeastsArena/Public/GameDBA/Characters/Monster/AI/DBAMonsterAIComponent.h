// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 怪物AI组件

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameDBA/Characters/Monster/AI/EMonsterAIState.h"
#include "GameDBA/Characters/Monster/AI/FAggroInfo.h"
#include "DBAMonsterAIComponent.generated.h"

class ADBAZodiacCharacterBase;
class UDBADamageCalculator;

/**
 * UDBAMonsterAIComponent
 * 怪物AI状态机组件
 * 负责管理AI状态转换、仇恨系统和巡逻
 */
UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class DIVINEBEASTSARENA_API UDBAMonsterAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBAMonsterAIComponent();

protected:
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// ===== AI状态 =====
	/** 当前AI状态 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI State")
	EMonsterAIState CurrentState = EMonsterAIState::Idle;

	/** 当前攻击目标 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI State")
	AActor* CurrentTarget = nullptr;

	// ===== AI配置 =====
	/** 检测范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	float DetectionRadius = 500.0f;

	/** 攻击范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	float AttackRange = 150.0f;

	/** 最大追击距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	float ChaseMaxDistance = 1000.0f;

	/** 巡逻速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	float PatrolSpeed = 200.0f;

	/** 追击速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	float ChaseSpeed = 400.0f;

	/** 攻击间隔 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	float AttackInterval = 1.5f;

	/** 基础攻击力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	float BaseAttackDamage = 20.0f;

	// ===== 巡逻路径配置 =====
	/** 巡逻路径点 (代码配置) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Patrol")
	TArray<FVector> PatrolPoints_CPP;

	/** 巡逻路径点Actor类 (编辑器放置) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Patrol")
	TSubclassOf<AActor> PatrolPointActorClass;

	/** 是否循环巡逻 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Patrol")
	bool bLoopPatrol = true;

	// ===== 仇恨系统 =====
	/** 仇恨列表 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "AI Aggro")
	TArray<FAggroInfo> AggroList;

	/** Idle到Patrol的延迟时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
	float IdleToPatrolDelay = 3.0f;

	/** 到达目标判定半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config", meta = (UIMin = 10.0, UIMax = 200.0))
	float AcceptanceRadius = 50.0f;

public:
	// ===== 导航 =====
	/** 移动到目标位置 */
	UFUNCTION(BlueprintCallable, Category = "AI|Movement")
	void MoveToLocation(FVector Destination);

	/** 移动到目标Actor */
	UFUNCTION(BlueprintCallable, Category = "AI|Movement")
	void MoveToActor(AActor* Target);

	/** 停止移动 */
	UFUNCTION(BlueprintCallable, Category = "AI|Movement")
	void StopMovement();

	/** 是否正在移动 */
	UFUNCTION(BlueprintCallable, Category = "AI|Movement")
	bool IsMoving() const;

public:
	// ===== 状态转换 =====
	/** 切换到新状态 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void TransitionTo(EMonsterAIState NewState);

	// ===== 目标检测 =====
	/** 查找最近敌人 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void FindTarget();

	/** 清除目标 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void ClearTarget();

	// ===== 攻击 =====
	/** 执行攻击 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void AttackTarget();

	/** 获取是否在攻击范围 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	bool IsInAttackRange() const;

	/** 获取是否在检测范围 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	bool IsInDetectionRange() const;

	// ===== 仇恨管理 =====
	/** 增加仇恨 */
	UFUNCTION(BlueprintCallable, Category = "AI|Aggro")
	void AddAggro(AActor* Target, float Amount);

	/** 移除仇恨 */
	UFUNCTION(BlueprintCallable, Category = "AI|Aggro")
	void RemoveAggro(AActor* Target);

	/** 获取仇恨最高的目标 */
	UFUNCTION(BlueprintCallable, Category = "AI|Aggro")
	AActor* GetTopAggroTarget();

	/** 清空仇恨列表 */
	UFUNCTION(BlueprintCallable, Category = "AI|Aggro")
	void ClearAggroList();

	// ===== 巡逻 =====
	/** 获取巡逻点数量 */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	int32 GetPatrolPointCount() const;

	/** 获取指定巡逻点 */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	FVector GetPatrolPoint(int32 Index) const;

	/** 获取下一个巡逻点 */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	FVector GetNextPatrolPoint();

	/** 获取出生点 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	FVector GetSpawnLocation() const { return SpawnLocation; }

	/** 设置出生点 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetSpawnLocation(FVector Location);

protected:
	// ===== 内部方法 =====
	/** 更新仇恨列表 (衰减和清理) */
	UFUNCTION(BlueprintCallable, Category = "AI|Aggro")
	void UpdateAggroList();

	/** 检查是否有视线 */
	bool HasLineOfSightTo(FVector TargetLocation) const;

	/** 查找所有可攻击目标 */
	TArray<AActor*> FindAllValidTargets() const;

	/** 刷新仇恨目标 */
	void RefreshAggroTarget();

private:
	/** 当前巡逻点索引 */
	UPROPERTY(Transient)
	int32 CurrentPatrolIndex = 0;

	/** 出生点位置 */
	UPROPERTY(Transient)
	FVector SpawnLocation = FVector::ZeroVector;

	/** 最后攻击时间 */
	UPROPERTY(Transient)
	float LastAttackTime = 0.0f;

	/** 最后更新仇恨时间 */
	UPROPERTY(Transient)
	float LastAggroUpdateTime = 0.0f;

public:
	// ===== 复制通知 =====
	UFUNCTION()
	virtual void OnRep_CurrentState(EMonsterAIState OldState);

	UFUNCTION()
	virtual void OnRep_CurrentTarget(AActor* OldTarget);
};
