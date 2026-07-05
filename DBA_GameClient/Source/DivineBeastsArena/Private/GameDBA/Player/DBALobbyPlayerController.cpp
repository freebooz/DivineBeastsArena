// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Player/DBALobbyPlayerController.h"

#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Character/Monster/DBAMonsterBase.h"
#include "GameDBA/Character/Monster/DBALobbyTrainingMonster.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/DBAGameUIManager.h"
#include "GameDBA/UI/Common/UDBASoftwareCursorWidget.h"
#include "Blueprint/UserWidget.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

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
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = false;
	bEnableTouchEvents = true;
	bEnableTouchOverEvents = false;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	CurrentMouseCursor = EMouseCursor::Default;
	DefaultClickTraceChannel = ECC_Pawn;

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
		MouseYawSensitivityScale = FMath::Max(MouseYawSensitivityScale, 21.0f);
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		SetInputMode(InputMode);
		SetShowMouseCursor(true);
		EnsureCustomSoftwareCursor();
		bEnableClickEvents = true;
		bEnableTouchEvents = true;
		if (APawn* ControlledPawn = GetPawn())
		{
			InitializeLobbyCameraForPawn(ControlledPawn);
			SetViewTarget(ControlledPawn);
			UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] BeginPlay 已将视角目标设置为角色：%s"),
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
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		SetInputMode(InputMode);
		SetShowMouseCursor(true);
		EnsureCustomSoftwareCursor();
		bEnableClickEvents = true;
		InitializeLobbyCameraForPawn(InPawn);
		SetViewTarget(InPawn);
		UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] OnPossess 已将视角目标设置为角色：%s"),
			*InPawn->GetName());
		RequestLobbyHUDForLocalController(this);
	}
}

void ADBALobbyPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	if (IsLocalController() && P)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		SetInputMode(InputMode);
		SetShowMouseCursor(true);
		EnsureCustomSoftwareCursor();
		bEnableClickEvents = true;
		InitializeLobbyCameraForPawn(P);
		SetViewTarget(P);
		UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] AcknowledgePossession 已将视角目标设置为角色：%s"),
			*P->GetName());
		RequestLobbyHUDForLocalController(this);
	}
}

void ADBALobbyPlayerController::EnsureCustomSoftwareCursor()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!SoftwareCursorWidget)
	{
		SoftwareCursorWidget = CreateWidget<UDBASoftwareCursorWidget>(this, UDBASoftwareCursorWidget::StaticClass());
	}
	if (!SoftwareCursorWidget)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBALobbyPlayerController] 创建软件鼠标指针控件失败。"));
		return;
	}

	SetMouseCursorWidget(EMouseCursor::Default, SoftwareCursorWidget);
	SetMouseCursorWidget(EMouseCursor::Crosshairs, SoftwareCursorWidget);
	SetMouseCursorWidget(EMouseCursor::Hand, SoftwareCursorWidget);
	CurrentMouseCursor = EMouseCursor::Default;
	DefaultMouseCursor = EMouseCursor::Default;
	SetShowMouseCursor(true);
	UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] 已应用自定义软件鼠标指针：%s"),
		*SoftwareCursorWidget->GetName());
}

void ADBALobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent)
	{
		return;
	}

	if (!IsLocalController())
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
	InputComponent->BindAction(TEXT("Skill02"), IE_Pressed, this, &ADBALobbyPlayerController::HandleSkill02Pressed);
	InputComponent->BindAction(TEXT("Skill03"), IE_Pressed, this, &ADBALobbyPlayerController::HandleSkill03Pressed);
	InputComponent->BindAction(TEXT("Skill04"), IE_Pressed, this, &ADBALobbyPlayerController::HandleSkill04Pressed);
	InputComponent->BindAction(TEXT("Ultimate"), IE_Pressed, this, &ADBALobbyPlayerController::HandleUltimatePressed);
	InputComponent->BindAction(TEXT("Skill06"), IE_Pressed, this, &ADBALobbyPlayerController::HandleSkill06Pressed);
	InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ADBALobbyPlayerController::HandleSkill01Pressed);
	InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &ADBALobbyPlayerController::HandleSkill06Pressed);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ADBALobbyPlayerController::HandleLeftMousePressed);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ADBALobbyPlayerController::HandleLeftMouseReleased);
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ADBALobbyPlayerController::HandleRightMousePressed);
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &ADBALobbyPlayerController::HandleRightMouseReleased);
	InputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &ADBALobbyPlayerController::HandleMouseScrollUp);
	InputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &ADBALobbyPlayerController::HandleMouseScrollDown);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ADBALobbyPlayerController::HandleEscapePressed);
	InputComponent->BindKey(EKeys::B, IE_Pressed, this, &ADBALobbyPlayerController::HandleInventoryPressed);
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
	float ForwardValue = !FMath::IsNearlyZero(AnalogMoveForwardAxis) ? AnalogMoveForwardAxis : TaggedMove.X;
	const float RightValue = !FMath::IsNearlyZero(AnalogMoveRightAxis) ? AnalogMoveRightAxis : TaggedMove.Y;
	if (bLeftMouseLookHeld && bRightMouseLookHeld)
	{
		ForwardValue = FMath::Max(ForwardValue, 1.0f);
	}
	ApplyMovementInput(ForwardValue, RightValue);
	RefreshMouseLookCaptureMode();
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

void ADBALobbyPlayerController::ToggleGameSettingsPanel()
{
	CancelMouseLookCapture();
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->ToggleGameSettings();
		}
	}
}

void ADBALobbyPlayerController::ToggleInventoryPanel()
{
	CancelMouseLookCapture();
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->ToggleInventory();
		}
	}
}

void ADBALobbyPlayerController::CancelMouseLookCapture()
{
	bLeftMouseLookHeld = false;
	bRightMouseLookHeld = false;
	bLeftMouseClickCandidate = false;
	LeftMouseDragDistance = 0.0f;
	SetMouseLookCaptureActive(false);
	ConfigurePawnForRightMouseLook(false);
}

void ADBALobbyPlayerController::SetMouseLookSensitivityValue(float NewSensitivity)
{
	MouseLookSensitivity = FMath::Clamp(NewSensitivity, 0.02f, 0.80f);
}

void ADBALobbyPlayerController::SetCameraDistanceValue(float NewDistance)
{
	ADBAZodiacCharacterBase* ZodiacPawn = Cast<ADBAZodiacCharacterBase>(GetPawn());
	USpringArmComponent* CameraBoom = ZodiacPawn ? ZodiacPawn->GetLobbyCameraBoom() : nullptr;
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = FMath::Clamp(NewDistance, MinCameraDistance, MaxCameraDistance);
	}
}

float ADBALobbyPlayerController::GetCameraDistanceValue() const
{
	const ADBAZodiacCharacterBase* ZodiacPawn = Cast<ADBAZodiacCharacterBase>(GetPawn());
	const USpringArmComponent* CameraBoom = ZodiacPawn ? ZodiacPawn->GetLobbyCameraBoom() : nullptr;
	return CameraBoom ? CameraBoom->TargetArmLength : 520.0f;
}

void ADBALobbyPlayerController::TurnAxis(float Value)
{
	CachedTurnAxis = Value;
}

void ADBALobbyPlayerController::HandleLeftMousePressed()
{
	SaveMouseLookCursorPosition();
	bLeftMouseLookHeld = true;
	bLeftMouseClickCandidate = true;
	LeftMouseDragDistance = 0.0f;
	RefreshMouseLookCaptureMode();
}

void ADBALobbyPlayerController::HandleLeftMouseReleased()
{
	const bool bIsRightMouseDown = bRightMouseLookHeld || IsInputKeyDown(EKeys::RightMouseButton);
	const bool bShouldSelectClickedTarget = bLeftMouseClickCandidate && !bIsRightMouseDown;
	bLeftMouseLookHeld = false;
	bLeftMouseClickCandidate = false;
	LeftMouseDragDistance = 0.0f;
	RefreshMouseLookCaptureMode();
	if (bShouldSelectClickedTarget)
	{
		HandleSelectTargetPressed();
	}
}

void ADBALobbyPlayerController::HandleRightMousePressed()
{
	SaveMouseLookCursorPosition();
	bRightMouseLookHeld = true;
	bLeftMouseClickCandidate = false;
	RefreshMouseLookCaptureMode();
}

void ADBALobbyPlayerController::HandleRightMouseReleased()
{
	bRightMouseLookHeld = false;
	RefreshMouseLookCaptureMode();
}

void ADBALobbyPlayerController::HandleMouseScrollUp()
{
	ApplyCameraZoom(1.0f);
}

void ADBALobbyPlayerController::HandleMouseScrollDown()
{
	ApplyCameraZoom(-1.0f);
}

void ADBALobbyPlayerController::HandleEscapePressed()
{
	ToggleGameSettingsPanel();
}

void ADBALobbyPlayerController::HandleInventoryPressed()
{
	ToggleInventoryPanel();
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
	CastEquippedSkillSlot(1);
}

void ADBALobbyPlayerController::HandleSkill02Pressed()
{
	CastEquippedSkillSlot(2);
}

void ADBALobbyPlayerController::HandleSkill03Pressed()
{
	CastEquippedSkillSlot(3);
}

void ADBALobbyPlayerController::HandleSkill04Pressed()
{
	CastEquippedSkillSlot(4);
}

void ADBALobbyPlayerController::HandleUltimatePressed()
{
	CastEquippedSkillSlot(5);
}

void ADBALobbyPlayerController::HandleSkill06Pressed()
{
	CastEquippedSkillSlot(6);
}

void ADBALobbyPlayerController::CastEquippedSkillSlot(int32 SkillSlot)
{
	if (!IsLocalController())
	{
		return;
	}

	if (ADBAZodiacCharacterBase* ZodiacPawn = Cast<ADBAZodiacCharacterBase>(GetPawn()))
	{
		AActor* Target = SelectedAttackTarget.IsValid() ? SelectedAttackTarget.Get() : nullptr;
		if (ADBALobbyTrainingMonster* TrainingTarget = Cast<ADBALobbyTrainingMonster>(Target))
		{
			if (TrainingTarget->GetHealthPercent() <= 0.0f)
			{
				TrainingTarget->SetLobbySelected(false);
				SelectedAttackTarget.Reset();
				Target = nullptr;
			}
		}
		if (!Target)
		{
			Target = ResolveAutoAttackTarget();
			if (ADBALobbyTrainingMonster* AutoTrainingTarget = Cast<ADBALobbyTrainingMonster>(Target))
			{
				if (ADBALobbyTrainingMonster* PreviousTarget = Cast<ADBALobbyTrainingMonster>(SelectedAttackTarget.Get()))
				{
					PreviousTarget->SetLobbySelected(false);
				}
				SelectedAttackTarget = AutoTrainingTarget;
				AutoTrainingTarget->SetLobbySelected(true);
			}
		}
		if (Target)
		{
			ZodiacPawn->CastEquippedSkillAtTarget(SkillSlot, Target);
		}
		else
		{
			ZodiacPawn->CastEquippedSkill(SkillSlot);
		}
		UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] 触发装配技能：槽位=%d 角色=%s 目标=%s"),
			SkillSlot,
			*ZodiacPawn->GetName(),
			*GetNameSafe(Target));
	}
}

void ADBALobbyPlayerController::HandleSelectTargetPressed()
{
	AActor* Target = ResolveClickedAttackTarget();
	if (!Target)
	{
		return;
	}

	if (ADBALobbyTrainingMonster* PreviousTarget = Cast<ADBALobbyTrainingMonster>(SelectedAttackTarget.Get()))
	{
		PreviousTarget->SetLobbySelected(false);
	}

	SelectedAttackTarget = Target;
	if (ADBALobbyTrainingMonster* NewTarget = Cast<ADBALobbyTrainingMonster>(Target))
	{
		NewTarget->SetLobbySelected(true);
	}
	UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] 已选中攻击目标：%s"), *GetNameSafe(Target));
}

AActor* ADBALobbyPlayerController::ResolveAutoAttackTarget() const
{
	const APawn* ControlledPawn = GetPawn();
	const UWorld* World = GetWorld();
	if (!ControlledPawn || !World)
	{
		return nullptr;
	}

	const FVector PawnLocation = ControlledPawn->GetActorLocation();
	const FVector Forward = ControlledPawn->GetActorForwardVector().GetSafeNormal();
	constexpr float MaxAutoTargetDistance = 1800.0f;
	constexpr float MinForwardDot = 0.12f;

	AActor* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	for (TActorIterator<ADBALobbyTrainingMonster> It(World); It; ++It)
	{
		ADBALobbyTrainingMonster* Candidate = *It;
		if (!Candidate || Candidate->GetHealthPercent() <= 0.0f)
		{
			continue;
		}

		const FVector ToTarget = Candidate->GetActorLocation() - PawnLocation;
		const float DistanceSquared = ToTarget.SizeSquared2D();
		if (DistanceSquared > FMath::Square(MaxAutoTargetDistance))
		{
			continue;
		}

		const FVector Direction = FVector(ToTarget.X, ToTarget.Y, 0.0f).GetSafeNormal();
		const float ForwardDot = FVector::DotProduct(Forward, Direction);
		if (ForwardDot < MinForwardDot)
		{
			continue;
		}

		const float Score = DistanceSquared * (1.35f - FMath::Clamp(ForwardDot, 0.0f, 1.0f));
		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void ADBALobbyPlayerController::LookUpAxis(float Value)
{
	CachedLookUpAxis = Value;
}

void ADBALobbyPlayerController::ApplyMovementInput(float ForwardValue, float RightValue)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	const FRotator ControlRot = GetControlRotation();
	const bool bUseMouseFacing = bRightMouseLookHeld || IsInputKeyDown(EKeys::RightMouseButton);
	const float MovementYaw = bUseMouseFacing ? ControlRot.Yaw : ControlledPawn->GetActorRotation().Yaw;
	const FRotator YawRot(0.0f, MovementYaw, 0.0f);
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
	const bool bIsLeftLooking = bLeftMouseLookHeld || IsInputKeyDown(EKeys::LeftMouseButton);
	const bool bIsRightLooking = bRightMouseLookHeld || IsInputKeyDown(EKeys::RightMouseButton);
	if (!bIsLeftLooking && !bIsRightLooking)
	{
		return;
	}

	float MouseDeltaX = 0.0f;
	float MouseDeltaY = 0.0f;
	GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

	const float YawSensitivity = MouseLookSensitivity * MouseYawSensitivityScale;
	const float PitchSensitivity = MouseLookSensitivity * MousePitchSensitivityScale;
	float YawInput = MouseDeltaX * YawSensitivity;
	float PitchInput = MouseDeltaY * PitchSensitivity;
	if (FMath::IsNearlyZero(MouseDeltaX) && FMath::IsNearlyZero(MouseDeltaY))
	{
		YawInput = CachedTurnAxis * YawSensitivity;
		PitchInput = -CachedLookUpAxis * PitchSensitivity;
	}

	const float MouseDragDistanceThisFrame = FVector2D(MouseDeltaX, MouseDeltaY).Size();
	if (bLeftMouseClickCandidate)
	{
		LeftMouseDragDistance += MouseDragDistanceThisFrame;
		if (LeftMouseDragDistance > FMath::Max(MouseClickDragThreshold, 0.0f))
		{
			bLeftMouseClickCandidate = false;
		}
	}

	ApplyCameraInput(YawInput, PitchInput, bIsRightLooking);
	CachedTurnAxis = 0.0f;
	CachedLookUpAxis = 0.0f;
}

void ADBALobbyPlayerController::ApplyCameraInput(float YawInput, float PitchInput, bool bTurnCharacter)
{
	if (FMath::IsNearlyZero(YawInput) && FMath::IsNearlyZero(PitchInput))
	{
		if (bTurnCharacter)
		{
			FacePawnToControlYaw();
		}
		return;
	}

	FRotator NewControlRotation = GetControlRotation();
	NewControlRotation.Yaw = FRotator::NormalizeAxis(NewControlRotation.Yaw + YawInput);
	const float CurrentPitch = FRotator::NormalizeAxis(NewControlRotation.Pitch);
	NewControlRotation.Pitch = FMath::Clamp(CurrentPitch + PitchInput, MinCameraPitch, MaxCameraPitch);
	NewControlRotation.Roll = 0.0f;
	SetControlRotation(NewControlRotation);

	if (bTurnCharacter)
	{
		FacePawnToControlYaw();
	}
}

void ADBALobbyPlayerController::ApplyCameraZoom(float WheelSteps)
{
	ADBAZodiacCharacterBase* ZodiacPawn = Cast<ADBAZodiacCharacterBase>(GetPawn());
	USpringArmComponent* CameraBoom = ZodiacPawn ? ZodiacPawn->GetLobbyCameraBoom() : nullptr;
	if (!CameraBoom)
	{
		return;
	}

	const float ZoomDelta = WheelSteps * CameraZoomStep;
	CameraBoom->TargetArmLength = FMath::Clamp(
		CameraBoom->TargetArmLength - ZoomDelta,
		MinCameraDistance,
		MaxCameraDistance);
}

void ADBALobbyPlayerController::RefreshMouseLookCaptureMode()
{
	const bool bShouldCapture = bLeftMouseLookHeld || bRightMouseLookHeld;
	SetMouseLookCaptureActive(bShouldCapture);
	ConfigurePawnForRightMouseLook(bRightMouseLookHeld || IsInputKeyDown(EKeys::RightMouseButton));
}

void ADBALobbyPlayerController::SetMouseLookCaptureActive(bool bActive)
{
	if (bMouseLookCaptureActive == bActive)
	{
		return;
	}

	bMouseLookCaptureActive = bActive;
	if (bActive)
	{
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		SetInputMode(InputMode);
		CurrentMouseCursor = EMouseCursor::Default;
		DefaultMouseCursor = EMouseCursor::Default;
		if (!SoftwareCursorWidget)
		{
			EnsureCustomSoftwareCursor();
		}
		SetShowMouseCursor(false);
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
		return;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(InputMode);
	bEnableClickEvents = true;
	bEnableMouseOverEvents = false;
	CurrentMouseCursor = EMouseCursor::Default;
	DefaultMouseCursor = EMouseCursor::Default;
	SetShowMouseCursor(true);
	if (!SoftwareCursorWidget)
	{
		EnsureCustomSoftwareCursor();
	}
	RestoreMouseLookCursorPosition();
}

void ADBALobbyPlayerController::ConfigurePawnForRightMouseLook(bool bActive)
{
	bPawnUsingRightMouseLook = bActive;

	if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
	{
		ControlledCharacter->bUseControllerRotationYaw = bActive;
		if (UCharacterMovementComponent* Movement = ControlledCharacter->GetCharacterMovement())
		{
			Movement->bOrientRotationToMovement = false;
			Movement->bUseControllerDesiredRotation = bActive;
		}
	}

	if (bActive)
	{
		FacePawnToControlYaw();
	}
}

void ADBALobbyPlayerController::FacePawnToControlYaw()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	const FRotator DesiredRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	ControlledPawn->SetActorRotation(DesiredRotation);
}

void ADBALobbyPlayerController::InitializeLobbyCameraForPawn(APawn* ControlledPawn)
{
	if (!ControlledPawn)
	{
		return;
	}

	const FRotator PawnRotation = ControlledPawn->GetActorRotation();
	FRotator CameraRotation = GetControlRotation();
	const float NormalizedPitch = FRotator::NormalizeAxis(CameraRotation.Pitch);
	if (FMath::IsNearlyZero(NormalizedPitch, 1.0f))
	{
		CameraRotation.Pitch = FMath::Clamp(DefaultCameraPitch, MinCameraPitch, MaxCameraPitch);
	}
	CameraRotation.Yaw = PawnRotation.Yaw;
	CameraRotation.Roll = 0.0f;
	SetControlRotation(CameraRotation);
	ConfigurePawnForRightMouseLook(false);
}

void ADBALobbyPlayerController::SaveMouseLookCursorPosition()
{
	if (bMouseLookCaptureActive || bHasSavedMouseLookCursorPosition)
	{
		return;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;
	if (GetMousePosition(MouseX, MouseY))
	{
		SavedMouseLookCursorPosition = FVector2D(MouseX, MouseY);
		bHasSavedMouseLookCursorPosition = true;
	}
}

void ADBALobbyPlayerController::RestoreMouseLookCursorPosition()
{
	if (!bRestoreCursorAfterMouseLook || !bHasSavedMouseLookCursorPosition)
	{
		bHasSavedMouseLookCursorPosition = false;
		return;
	}

	SetMouseLocation(
		FMath::RoundToInt(SavedMouseLookCursorPosition.X),
		FMath::RoundToInt(SavedMouseLookCursorPosition.Y));
	bHasSavedMouseLookCursorPosition = false;
}

AActor* ADBALobbyPlayerController::ResolveClickedAttackTarget() const
{
	FHitResult Hit;
	if (!GetHitResultUnderCursor(ECC_Pawn, true, Hit))
	{
		return nullptr;
	}

	AActor* HitActor = Hit.GetActor();
	if (Cast<ADBAMonsterBase>(HitActor))
	{
		return HitActor;
	}

	return nullptr;
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
		ApplyCameraInput(Delta.X * TouchLookSensitivity * MouseYawSensitivityScale, Delta.Y * TouchLookSensitivity * MousePitchSensitivityScale, true);
	}
}
