// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Character/Monster/DBAMonsterBase.h"
#include "DBALobbyTrainingMonster.generated.h"

class UAnimationAsset;
class UPointLightComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class UDBALobbyMonsterHealthBarWidget;
class UDBALobbyFloatingDamageComponent;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBALobbyTrainingMonster : public ADBAMonsterBase
{
	GENERATED_BODY()

public:
	ADBALobbyTrainingMonster();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Monster")
	void ConfigureLobbyMonster(int32 MonsterIndex);

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Monster")
	void SetLobbySelected(bool bSelected);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnRep_CurrentHealth() override;
	virtual void HandleMonsterDefeated(AActor* DamageCauser) override;

private:
	void ApplyLobbyMonsterVisuals();
	void UpdateHealthBar();
	void RespawnAfterDefeat();
	void ConfigurePatrolRoute();
	void UpdateLobbyPatrol(float DeltaSeconds);
	void AdvancePatrolTarget();
	void PlayLobbyMonsterAnimation(UAnimationAsset* AnimationAsset);
	void SetPatrolMovingAnimation(bool bMoving, bool bUpdateReplicatedState = true);
	void RefreshPatrolAnimationFromVelocity();

	UFUNCTION()
	void OnRep_ReplicatedPatrolMoving();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetDefeatedVisualState(bool bDefeated);

private:
	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster")
	int32 VisualIndex = 0;

	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster")
	float MeshScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster|Patrol")
	float PatrolRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster|Patrol")
	float PatrolSpeed = 115.0f;

	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster|Patrol")
	float PatrolAcceptanceRadius = 28.0f;

	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster|Patrol")
	float PatrolPauseSeconds = 0.6f;

	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster|Respawn", meta = (ClampMin = "0.1"))
	float RespawnDelaySeconds = 2.25f;

	UPROPERTY(Transient)
	TArray<FVector> PatrolPoints;

	UPROPERTY(Transient)
	int32 CurrentPatrolPointIndex = 0;

	UPROPERTY(Transient)
	float NextPatrolMoveTime = 0.0f;

	UPROPERTY(Transient)
	bool bPatrolRouteConfigured = false;

	UPROPERTY(Transient)
	bool bRespawning = false;

	UPROPERTY(Transient)
	FTransform InitialSpawnTransform;

	FTimerHandle RespawnTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedPatrolMoving, Transient)
	bool bReplicatedPatrolMoving = false;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> LobbyIdleAnimation;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> LobbyWalkAnimation;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationAsset> CurrentLobbyAnimation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Lobby|Monster", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Lobby|Monster", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDBALobbyFloatingDamageComponent> FloatingDamageComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Lobby|Monster", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SelectionRingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Lobby|Monster", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPointLightComponent> SelectionLightComponent;
};
