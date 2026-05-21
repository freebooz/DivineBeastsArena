// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
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
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

protected:
	void MoveForwardAxis(float Value);
	void MoveRightAxis(float Value);
	void TurnAxis(float Value);
	void LookUpAxis(float Value);
	void HandleLeftMousePressed();
	void HandleLeftMouseReleased();
	void HandleRightMousePressed();
	void HandleRightMouseReleased();
	void HandleMouseScrollUp();
	void HandleMouseScrollDown();
	void HandleMoveForwardPressed();
	void HandleMoveForwardReleased();
	void HandleMoveBackwardPressed();
	void HandleMoveBackwardReleased();
	void HandleMoveLeftPressed();
	void HandleMoveLeftReleased();
	void HandleMoveRightPressed();
	void HandleMoveRightReleased();
	void HandleSkill01Pressed();
	void HandleSelectTargetPressed();

	void ApplyMovementInput(float ForwardValue, float RightValue);
	void EnsureMovementInputTags();
	void RefreshMovementInputTagsFromKeyboard();
	void SetMovementTagActive(const FGameplayTag& Tag, bool bActive);
	FVector2D ResolveTaggedMovementInput() const;
	void ApplyMouseLook();
	void ApplyCameraInput(float YawInput, float PitchInput, bool bTurnCharacter);
	void ApplyCameraZoom(float WheelSteps);
	void RefreshMouseLookCaptureMode();
	void SetMouseLookCaptureActive(bool bActive);
	void ConfigurePawnForRightMouseLook(bool bActive);
	void FacePawnToControlYaw();
	void ApplyTouchLook();
	AActor* ResolveClickedAttackTarget() const;
	void EnsureCustomSoftwareCursor();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float MouseLookSensitivity = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float CameraZoomStep = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float MinCameraDistance = 280.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float MaxCameraDistance = 950.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float MinCameraPitch = -65.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float MaxCameraPitch = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float TouchLookSensitivity = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	bool bEnableTouchLook = true;

private:
	float AnalogMoveForwardAxis = 0.0f;
	float AnalogMoveRightAxis = 0.0f;
	float CachedTurnAxis = 0.0f;
	float CachedLookUpAxis = 0.0f;
	FGameplayTag MoveForwardTag;
	FGameplayTag MoveBackwardTag;
	FGameplayTag MoveLeftTag;
	FGameplayTag MoveRightTag;
	FGameplayTagContainer ActiveMovementInputTags;
	bool bJumpHeld = false;
	bool bLeftMouseLookHeld = false;
	bool bRightMouseLookHeld = false;
	bool bMouseLookCaptureActive = false;
	bool bPawnUsingRightMouseLook = false;
	bool bTouchLookWasPressed = false;
	FVector2D LastTouchLookPos = FVector2D::ZeroVector;
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> SelectedAttackTarget;

	UPROPERTY(Transient)
	TObjectPtr<class UDBASoftwareCursorWidget> SoftwareCursorWidget;
};
