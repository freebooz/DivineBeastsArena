// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DBALobbyPlayerController.generated.h"

/**
 * 大厅玩家控制器
 * - PC：WASD + 鼠标右键转向
 * - Mobile：支持轴向输入（虚拟摇杆映射到 MoveForward/MoveRight）
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API ADBALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADBALobbyPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

protected:
	void MoveForwardAxis(float Value);
	void MoveRightAxis(float Value);
	void TurnAxis(float Value);
	void LookUpAxis(float Value);

	void ApplyMovementInput(float ForwardValue, float RightValue);
	void ApplyDesktopFallbackInput();
	void ApplyMouseLookWhileRightButton();
	void ApplyTouchLook();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float MouseLookSensitivity = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float TouchLookSensitivity = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	bool bEnableTouchLook = true;

private:
	float CachedMoveForwardAxis = 0.0f;
	float CachedMoveRightAxis = 0.0f;
	float CachedTurnAxis = 0.0f;
	float CachedLookUpAxis = 0.0f;
	bool bJumpHeld = false;
	bool bTouchLookWasPressed = false;
	FVector2D LastTouchLookPos = FVector2D::ZeroVector;
};
