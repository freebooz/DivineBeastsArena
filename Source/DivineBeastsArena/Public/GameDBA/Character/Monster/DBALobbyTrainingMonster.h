// Copyright Freebooz Games, Inc. All Rights Reserved.

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

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Monster")
	void ConfigureLobbyMonster(int32 MonsterIndex);

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Monster")
	void SetLobbySelected(bool bSelected);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnRep_CurrentHealth() override;

private:
	void ApplyLobbyMonsterVisuals();
	void UpdateHealthBar();
	void ConfigurePatrolRoute();
	void UpdateLobbyPatrol(float DeltaSeconds);
	void AdvancePatrolTarget();
	void PlayLobbyMonsterAnimation(UAnimationAsset* AnimationAsset);
	void SetPatrolMovingAnimation(bool bMoving);

private:
	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster")
	int32 VisualIndex = 0;

	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster")
	float MeshScale = 0.92f;

	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster|Patrol")
	float PatrolRadius = 360.0f;

	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster|Patrol")
	float PatrolSpeed = 115.0f;

	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster|Patrol")
	float PatrolAcceptanceRadius = 28.0f;

	UPROPERTY(EditAnywhere, Category = "DBA|Lobby|Monster|Patrol")
	float PatrolPauseSeconds = 0.6f;

	UPROPERTY(Transient)
	TArray<FVector> PatrolPoints;

	UPROPERTY(Transient)
	int32 CurrentPatrolPointIndex = 0;

	UPROPERTY(Transient)
	float NextPatrolMoveTime = 0.0f;

	UPROPERTY(Transient)
	bool bPatrolRouteConfigured = false;

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
