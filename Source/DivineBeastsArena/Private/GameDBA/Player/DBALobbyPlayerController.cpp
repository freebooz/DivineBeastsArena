// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Player/DBALobbyPlayerController.h"

#include "GameDBA/Character/DBAZodiacCharacterBase.h"
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

	MoveForwardTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Move.Forward"), false);
	MoveBackwardTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Move.Backward"), false);
	MoveLeftTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Move.Left"), false);
	MoveRightTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Move.Right"), false);
}

void ADBALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		EnsureMovementInputTags();
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
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
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		SetInputMode(InputMode);
		SetShowMouseCursor(false);
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
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		SetInputMode(InputMode);
		SetShowMouseCursor(false);
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

	InputComponent->BindKey(EKeys::W, IE_Pressed, this, &ADBALobbyPlayerController::HandleMoveForwardPressed);
	InputComponent->BindKey(EKeys::W, IE_Released, this, &ADBALobbyPlayerController::HandleMoveForwardReleased);
	InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &ADBALobbyPlayerController::HandleMoveForwardPressed);
	InputComponent->BindKey(EKeys::Up, IE_Released, this, &ADBALobbyPlayerController::HandleMoveForwardReleased);
	InputComponent->BindKey(EKeys::S, IE_Pressed, this, &ADBALobbyPlayerController::HandleMoveBackwardPressed);
	InputComponent->BindKey(EKeys::S, IE_Released, this, &ADBALobbyPlayerController::HandleMoveBackwardReleased);
	InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &ADBALobbyPlayerController::HandleMoveBackwardPressed);
	InputComponent->BindKey(EKeys::Down, IE_Released, this, &ADBALobbyPlayerController::HandleMoveBackwardReleased);
	InputComponent->BindKey(EKeys::A, IE_Pressed, this, &ADBALobbyPlayerController::HandleMoveLeftPressed);
	InputComponent->BindKey(EKeys::A, IE_Released, this, &ADBALobbyPlayerController::HandleMoveLeftReleased);
	InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &ADBALobbyPlayerController::HandleMoveLeftPressed);
	InputComponent->BindKey(EKeys::Left, IE_Released, this, &ADBALobbyPlayerController::HandleMoveLeftReleased);
	InputComponent->BindKey(EKeys::D, IE_Pressed, this, &ADBALobbyPlayerController::HandleMoveRightPressed);
	InputComponent->BindKey(EKeys::D, IE_Released, this, &ADBALobbyPlayerController::HandleMoveRightReleased);
	InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &ADBALobbyPlayerController::HandleMoveRightPressed);
	InputComponent->BindKey(EKeys::Right, IE_Released, this, &ADBALobbyPlayerController::HandleMoveRightReleased);

	InputComponent->BindAction(TEXT("Skill01"), IE_Pressed, this, &ADBALobbyPlayerController::HandleSkill01Pressed);
	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ADBALobbyPlayerController::HandleSkill01Pressed);
}

void ADBALobbyPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (!IsLocalController())
	{
		return;
	}

	RefreshMovementInputTagsFromKeyboard();
	const FVector2D TaggedMove = ResolveTaggedMovementInput();
	const float ForwardValue = !FMath::IsNearlyZero(AnalogMoveForwardAxis) ? AnalogMoveForwardAxis : TaggedMove.X;
	const float RightValue = !FMath::IsNearlyZero(AnalogMoveRightAxis) ? AnalogMoveRightAxis : TaggedMove.Y;
	ApplyMovementInput(ForwardValue, RightValue);
	ApplyMouseLook();
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
	AnalogMoveForwardAxis = FMath::IsNearlyZero(Value, 0.05f) ? 0.0f : FMath::Clamp(Value, -1.0f, 1.0f);
}

void ADBALobbyPlayerController::MoveRightAxis(float Value)
{
	AnalogMoveRightAxis = FMath::IsNearlyZero(Value, 0.05f) ? 0.0f : FMath::Clamp(Value, -1.0f, 1.0f);
}

void ADBALobbyPlayerController::TurnAxis(float Value)
{
	CachedTurnAxis = Value;
	if (!FMath::IsNearlyZero(Value))
	{
		AddYawInput(Value);
	}
}

void ADBALobbyPlayerController::HandleMoveForwardPressed()
{
	SetMovementTagActive(MoveForwardTag, true);
}

void ADBALobbyPlayerController::HandleMoveForwardReleased()
{
	SetMovementTagActive(MoveForwardTag, false);
}

void ADBALobbyPlayerController::HandleMoveBackwardPressed()
{
	SetMovementTagActive(MoveBackwardTag, true);
}

void ADBALobbyPlayerController::HandleMoveBackwardReleased()
{
	SetMovementTagActive(MoveBackwardTag, false);
}

void ADBALobbyPlayerController::HandleMoveLeftPressed()
{
	SetMovementTagActive(MoveLeftTag, true);
}

void ADBALobbyPlayerController::HandleMoveLeftReleased()
{
	SetMovementTagActive(MoveLeftTag, false);
}

void ADBALobbyPlayerController::HandleMoveRightPressed()
{
	SetMovementTagActive(MoveRightTag, true);
}

void ADBALobbyPlayerController::HandleMoveRightReleased()
{
	SetMovementTagActive(MoveRightTag, false);
}

void ADBALobbyPlayerController::HandleSkill01Pressed()
{
	if (ADBAZodiacCharacterBase* ZodiacPawn = Cast<ADBAZodiacCharacterBase>(GetPawn()))
	{
		ZodiacPawn->CastLobbyFireball();
		UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] Skill01 pressed, casting lobby fireball from pawn: %s"),
			*ZodiacPawn->GetName());
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

void ADBALobbyPlayerController::RefreshMovementInputTagsFromKeyboard()
{
	EnsureMovementInputTags();
	SetMovementTagActive(MoveForwardTag, IsInputKeyDown(EKeys::W) || IsInputKeyDown(EKeys::Up));
	SetMovementTagActive(MoveBackwardTag, IsInputKeyDown(EKeys::S) || IsInputKeyDown(EKeys::Down));
	SetMovementTagActive(MoveLeftTag, IsInputKeyDown(EKeys::A) || IsInputKeyDown(EKeys::Left));
	SetMovementTagActive(MoveRightTag, IsInputKeyDown(EKeys::D) || IsInputKeyDown(EKeys::Right));
}

void ADBALobbyPlayerController::EnsureMovementInputTags()
{
	if (!MoveForwardTag.IsValid())
	{
		MoveForwardTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Move.Forward"), false);
	}
	if (!MoveBackwardTag.IsValid())
	{
		MoveBackwardTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Move.Backward"), false);
	}
	if (!MoveLeftTag.IsValid())
	{
		MoveLeftTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Move.Left"), false);
	}
	if (!MoveRightTag.IsValid())
	{
		MoveRightTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Move.Right"), false);
	}
}

void ADBALobbyPlayerController::SetMovementTagActive(const FGameplayTag& Tag, bool bActive)
{
	if (!Tag.IsValid())
	{
		return;
	}

	if (bActive)
	{
		ActiveMovementInputTags.AddTag(Tag);
	}
	else
	{
		ActiveMovementInputTags.RemoveTag(Tag);
	}
}

FVector2D ADBALobbyPlayerController::ResolveTaggedMovementInput() const
{
	float Forward = 0.0f;
	float Right = 0.0f;

	if (ActiveMovementInputTags.HasTagExact(MoveForwardTag))
	{
		Forward += 1.0f;
	}
	if (ActiveMovementInputTags.HasTagExact(MoveBackwardTag))
	{
		Forward -= 1.0f;
	}
	if (ActiveMovementInputTags.HasTagExact(MoveRightTag))
	{
		Right += 1.0f;
	}
	if (ActiveMovementInputTags.HasTagExact(MoveLeftTag))
	{
		Right -= 1.0f;
	}

	return FVector2D(FMath::Clamp(Forward, -1.0f, 1.0f), FMath::Clamp(Right, -1.0f, 1.0f));
}

void ADBALobbyPlayerController::ApplyMouseLook()
{
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
