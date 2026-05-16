// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Player/DBALobbyPlayerController.h"

#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/DBAGameUIManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
	bool IsLobbyGameplayWorld(const UWorld* World)
	{
		if (!World || !World->PersistentLevel)
		{
			return false;
		}

		const FString LevelPath = World->PersistentLevel->GetOutermost()->GetName();
		return LevelPath.Contains(TEXT("LobbyMap")) || LevelPath.Contains(TEXT("MainLobby"));
	}

	void RequestLobbyHUDForLocalController(APlayerController* PlayerController)
	{
		if (!PlayerController || !PlayerController->IsLocalController() || !IsLobbyGameplayWorld(PlayerController->GetWorld()))
		{
			return;
		}

		if (UGameInstance* GameInstance = PlayerController->GetGameInstance())
		{
			if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
			{
				UIManager->ShowMainLobby();
			}
		}
	}
}

ADBALobbyPlayerController::ADBALobbyPlayerController()
{
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	bEnableTouchEvents = true;
	bEnableTouchOverEvents = false;
}

void ADBALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		SetShowMouseCursor(false);
		bEnableTouchEvents = true;
		if (APawn* ControlledPawn = GetPawn())
		{
			SetViewTarget(ControlledPawn);
			UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] BeginPlay view target set to pawn: %s"),
				*ControlledPawn->GetName());
		}
		RequestLobbyHUDForLocalController(this);
	}
}

void ADBALobbyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (IsLocalController() && InPawn)
	{
		SetViewTarget(InPawn);
		UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] OnPossess view target set to pawn: %s"),
			*InPawn->GetName());
		RequestLobbyHUDForLocalController(this);
	}
}

void ADBALobbyPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	if (IsLocalController() && P)
	{
		SetViewTarget(P);
		UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] AcknowledgePossession view target set to pawn: %s"),
			*P->GetName());
		RequestLobbyHUDForLocalController(this);
	}
}

void ADBALobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent)
	{
		return;
	}

	// Keep both PC axis mappings and mobile virtual joystick mappings working.
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
	ApplyTouchLook();

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
	// Fallback when project axis mappings are unavailable.
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
	// Desktop mouse look while holding the right mouse button.
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

void ADBALobbyPlayerController::ApplyTouchLook()
{
	if (!bEnableTouchLook)
	{
		return;
	}

	float TouchX = 0.0f;
	float TouchY = 0.0f;
	bool bPressed = false;
	GetInputTouchState(ETouchIndex::Touch2, TouchX, TouchY, bPressed);
	if (!bPressed)
	{
		bTouchLookWasPressed = false;
		return;
	}

	const FVector2D CurrentPos(TouchX, TouchY);
	if (!bTouchLookWasPressed)
	{
		LastTouchLookPos = CurrentPos;
		bTouchLookWasPressed = true;
		return;
	}

	const FVector2D Delta = CurrentPos - LastTouchLookPos;
	LastTouchLookPos = CurrentPos;
	if (!Delta.IsNearlyZero())
	{
		AddYawInput(Delta.X * TouchLookSensitivity);
		AddPitchInput(-Delta.Y * TouchLookSensitivity);
	}
}
