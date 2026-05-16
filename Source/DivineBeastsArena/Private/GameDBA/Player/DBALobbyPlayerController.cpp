// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Player/DBALobbyPlayerController.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ADBALobbyPlayerController::ADBALobbyPlayerController()
{
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

void ADBALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		SetShowMouseCursor(false);
	}
}

void ADBALobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent)
	{
		return;
	}

	// 兼容项目输入映射（PC/移动端虚拟摇杆）
	InputComponent->BindAxis(TEXT("MoveForward"), this, &ADBALobbyPlayerController::MoveForwardAxis);
	InputComponent->BindAxis(TEXT("MoveRight"), this, &ADBALobbyPlayerController::MoveRightAxis);
	InputComponent->BindAxis(TEXT("Turn"), this, &ADBALobbyPlayerController::TurnAxis);
	InputComponent->BindAxis(TEXT("LookUp"), this, &ADBALobbyPlayerController::LookUpAxis);
}

void ADBALobbyPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (!IsLocalController())
	{
		return;
	}

	ApplyDesktopFallbackInput();
	ApplyMovementInput(CachedMoveForwardAxis, CachedMoveRightAxis);
	ApplyMouseLookWhileRightButton();

	if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
	{
		const bool bSpaceDown = IsInputKeyDown(EKeys::SpaceBar);
		if (bSpaceDown && !bJumpHeld)
		{
			ControlledCharacter->Jump();
			bJumpHeld = true;
		}
		else if (!bSpaceDown && bJumpHeld)
		{
			ControlledCharacter->StopJumping();
			bJumpHeld = false;
		}
	}
}

void ADBALobbyPlayerController::MoveForwardAxis(float Value)
{
	CachedMoveForwardAxis = Value;
}

void ADBALobbyPlayerController::MoveRightAxis(float Value)
{
	CachedMoveRightAxis = Value;
}

void ADBALobbyPlayerController::TurnAxis(float Value)
{
	CachedTurnAxis = Value;
	if (!FMath::IsNearlyZero(Value))
	{
		AddYawInput(Value);
	}
}

void ADBALobbyPlayerController::LookUpAxis(float Value)
{
	CachedLookUpAxis = Value;
	if (!FMath::IsNearlyZero(Value))
	{
		AddPitchInput(Value);
	}
}

void ADBALobbyPlayerController::ApplyMovementInput(float ForwardValue, float RightValue)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	const FRotator ControlRot = GetControlRotation();
	const FRotator YawRot(0.0f, ControlRot.Yaw, 0.0f);
	const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	if (!FMath::IsNearlyZero(ForwardValue))
	{
		ControlledPawn->AddMovementInput(ForwardDir, ForwardValue);
	}
	if (!FMath::IsNearlyZero(RightValue))
	{
		ControlledPawn->AddMovementInput(RightDir, RightValue);
	}
}

void ADBALobbyPlayerController::ApplyDesktopFallbackInput()
{
	// 当项目输入轴未配置时，提供PC键盘兜底输入。
	float Forward = CachedMoveForwardAxis;
	float Right = CachedMoveRightAxis;

	if (FMath::IsNearlyZero(Forward))
	{
		Forward = (IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f) + (IsInputKeyDown(EKeys::S) ? -1.0f : 0.0f);
	}
	if (FMath::IsNearlyZero(Right))
	{
		Right = (IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f) + (IsInputKeyDown(EKeys::A) ? -1.0f : 0.0f);
	}

	CachedMoveForwardAxis = FMath::Clamp(Forward, -1.0f, 1.0f);
	CachedMoveRightAxis = FMath::Clamp(Right, -1.0f, 1.0f);
}

void ADBALobbyPlayerController::ApplyMouseLookWhileRightButton()
{
	// PC端用右键按住时做镜头转向；移动端依赖触摸/轴输入，不走这里。
	if (!IsInputKeyDown(EKeys::RightMouseButton))
	{
		return;
	}

	float MouseDeltaX = 0.0f;
	float MouseDeltaY = 0.0f;
	GetInputMouseDelta(MouseDeltaX, MouseDeltaY);
	if (!FMath::IsNearlyZero(MouseDeltaX))
	{
		AddYawInput(MouseDeltaX * MouseLookSensitivity);
	}
	if (!FMath::IsNearlyZero(MouseDeltaY))
	{
		AddPitchInput(-MouseDeltaY * MouseLookSensitivity);
	}
}
