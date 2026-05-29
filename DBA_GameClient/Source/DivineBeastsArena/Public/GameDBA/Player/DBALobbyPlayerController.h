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
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "DBALobbyPlayerController.generated.h"

/**
 * 大厅玩家控制器
 * - PC：WASD + 魔兽世界式鼠标控制（左键转镜头、右键转角色、双键前进）
 * - Mobile：支持轴向输入（虚拟摇杆映射到 MoveForward/MoveRight）
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API ADBALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADBALobbyPlayerController();

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Input")
	void ToggleGameSettingsPanel();

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Input")
	void ToggleInventoryPanel();

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Input")
	void CancelMouseLookCapture();

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Camera")
	void SetMouseLookSensitivityValue(float NewSensitivity);

	UFUNCTION(BlueprintPure, Category = "DBA|Lobby|Camera")
	float GetMouseLookSensitivityValue() const { return MouseLookSensitivity; }

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Camera")
	void SetCameraDistanceValue(float NewDistance);

	UFUNCTION(BlueprintPure, Category = "DBA|Lobby|Camera")
	float GetCameraDistanceValue() const;

	UFUNCTION(BlueprintPure, Category = "DBA|Lobby|Camera")
	float GetMinCameraDistanceValue() const { return MinCameraDistance; }

	UFUNCTION(BlueprintPure, Category = "DBA|Lobby|Camera")
	float GetMaxCameraDistanceValue() const { return MaxCameraDistance; }

	UFUNCTION(BlueprintCallable, Category = "DBA|Lobby|Input")
	void CastEquippedSkillSlot(int32 SkillSlot);

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
	void HandleEscapePressed();
	void HandleInventoryPressed();
	void HandleMoveForwardPressed();
	void HandleMoveForwardReleased();
	void HandleMoveBackwardPressed();
	void HandleMoveBackwardReleased();
	void HandleMoveLeftPressed();
	void HandleMoveLeftReleased();
	void HandleMoveRightPressed();
	void HandleMoveRightReleased();
	void HandleSkill01Pressed();
	void HandleSkill02Pressed();
	void HandleSkill03Pressed();
	void HandleSkill04Pressed();
	void HandleUltimatePressed();
	void HandleSkill06Pressed();
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
	void InitializeLobbyCameraForPawn(APawn* ControlledPawn);
	void SaveMouseLookCursorPosition();
	void RestoreMouseLookCursorPosition();
	void ApplyTouchLook();
	AActor* ResolveClickedAttackTarget() const;
	AActor* ResolveAutoAttackTarget() const;
	void EnsureCustomSoftwareCursor();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float MouseLookSensitivity = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float MouseYawSensitivityScale = 21.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float MousePitchSensitivityScale = 1.35f;

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
	float DefaultCameraPitch = -18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	bool bRestoreCursorAfterMouseLook = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Lobby|Input")
	float MouseClickDragThreshold = 6.0f;

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
	bool bLeftMouseClickCandidate = false;
	bool bMouseLookCaptureActive = false;
	bool bPawnUsingRightMouseLook = false;
	bool bHasSavedMouseLookCursorPosition = false;
	bool bTouchLookWasPressed = false;
	float LeftMouseDragDistance = 0.0f;
	FVector2D SavedMouseLookCursorPosition = FVector2D::ZeroVector;
	FVector2D LastTouchLookPos = FVector2D::ZeroVector;
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> SelectedAttackTarget;

	UPROPERTY(Transient)
	TObjectPtr<class UDBASoftwareCursorWidget> SoftwareCursorWidget;
};
