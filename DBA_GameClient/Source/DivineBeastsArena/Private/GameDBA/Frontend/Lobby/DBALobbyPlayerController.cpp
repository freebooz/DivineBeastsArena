// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Frontend/Lobby/DBALobbyPlayerController.h"

#include "GameDBA/Framework/GameInstance/DBAGameInstance.h"
#include "GameDBA/Gameplay/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/Gameplay/Input/Components/DBAEnhancedInputComponent.h"
#include "GameDBA/Characters/DBAZodiacCharacterBase.h"
#include "GameDBA/Characters/IDBACharacterRef.h"
#include "GameDBA/Characters/Monster/DBAMonsterBase.h"
#include "GameDBA/Characters/Monster/DBALobbyTrainingMonster.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Framework/Replication/RPC/DBARpcHandler.h"
#include "GameDBA/UI/Controllers/DBAGameUIManager.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterPresentationActor.h"
#include "GameDBA/UI/Widgets/Common/UDBASoftwareCursorWidget.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

// P1-4 改造：Enhanced Input 系统头文件
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameDBA/Gameplay/Input/Configuration/DBALobbyInputConfigDataAsset.h"
#include "GameDBA/Gameplay/Input/Configuration/DBAEnhancedInputDeveloperSettings.h"
#include "InputMappingContext.h"

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

	bool IsFrontendLoginWorld(const UWorld* World)
	{
		if (!World || !World->PersistentLevel)
		{
			return false;
		}

		const FString LevelPath = World->PersistentLevel->GetOutermost()->GetName();
		return LevelPath.Contains(TEXT("FrontendMap"));
	}

	bool IsCharacterPresentationFlowActive(const APlayerController* PlayerController)
	{
		if (!PlayerController)
		{
			return false;
		}

		const UGameInstance* GameInstance = PlayerController->GetGameInstance();
		const UDBAFrontendFlowSubsystem* LoginFlow = GameInstance
			? GameInstance->GetSubsystem<UDBAFrontendFlowSubsystem>()
			: nullptr;
		if (!LoginFlow)
		{
			return false;
		}

		const EDBALoginFlowState FlowState = LoginFlow->GetFlowState();
		return FlowState == EDBALoginFlowState::CharacterSelecting
			|| FlowState == EDBALoginFlowState::CharacterCreating;
	}

	bool ShouldLockCameraToLobbyPawn(const APlayerController* PlayerController)
	{
		if (!PlayerController)
		{
			return false;
		}

		const UWorld* World = PlayerController->GetWorld();
		// FrontendMap 环境已被清空：若再把视角锁回 Pawn/Controller，会立刻只剩黑底+UI。
		if (IsFrontendLoginWorld(World) || IsCharacterPresentationFlowActive(PlayerController))
		{
			return false;
		}

		if (const AActor* ViewTarget = PlayerController->GetViewTarget())
		{
			if (ViewTarget->IsA<ADBACharacterPresentationActor>())
			{
				return false;
			}
		}

		return IsLobbyGameplayWorld(World);
	}

	void SuppressLobbyPawnCameraSteal(APlayerController* PlayerController)
	{
		if (!PlayerController || ShouldLockCameraToLobbyPawn(PlayerController))
		{
			return;
		}

		// 必须在 Super::OnPossess / AcknowledgePossession 之前关闭，否则引擎会先把 ViewTarget 锁到 Pawn。
		PlayerController->bAutoManageActiveCameraTarget = false;
	}

	void RestorePresentationCameraIfNeeded(APlayerController* PlayerController)
	{
		if (!PlayerController || ShouldLockCameraToLobbyPawn(PlayerController))
		{
			return;
		}

		UWorld* World = PlayerController->GetWorld();
		if (!World)
		{
			return;
		}

		// 只拉回已存在的舞台，避免在 Possess 早期误 Spawn。
		ADBACharacterPresentationActor* Stage = nullptr;
		for (TActorIterator<ADBACharacterPresentationActor> It(World); It; ++It)
		{
			if (IsValid(*It))
			{
				Stage = *It;
				break;
			}
		}
		if (!Stage)
		{
			return;
		}

		const AActor* ViewTarget = PlayerController->GetViewTarget();
		if (ViewTarget == Stage)
		{
			return;
		}

		Stage->ActivatePresentationCamera(PlayerController, 0.0f);
		UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] Possess 抢镜后已拉回选角展示相机。Stage=%s Prev=%s"),
			*Stage->GetName(),
			ViewTarget ? *ViewTarget->GetName() : TEXT("None"));
	}

	bool IsClientSideEnemyCandidate(const AActor* OwnerActor, AActor* Candidate)
	{
		if (!OwnerActor || !Candidate || Candidate == OwnerActor)
		{
			return false;
		}

		if (Cast<ADBALobbyTrainingMonster>(Candidate))
		{
			return true;
		}

		if (OwnerActor->Implements<UIDBACharacterRef>() && Candidate->Implements<UIDBACharacterRef>())
		{
			const TScriptInterface<IIDBACharacterRef> OwnerRef(const_cast<AActor*>(OwnerActor));
			const TScriptInterface<IIDBACharacterRef> CandidateRef(Candidate);
			return OwnerRef->GetTeamID() != CandidateRef->GetTeamID();
		}

		return false;
	}

	void RequestLobbyHUDForLocalController(APlayerController* PlayerController)
	{
		if (!PlayerController || !PlayerController->IsLocalController() || !IsLobbyGameplayWorld(PlayerController->GetWorld()))
		{
			return;
		}

		UGameInstance* GameInstance = PlayerController->GetGameInstance();
		UDBAFrontendFlowSubsystem* LoginFlow = GameInstance
			? GameInstance->GetSubsystem<UDBAFrontendFlowSubsystem>()
			: nullptr;
		if (LoginFlow && LoginFlow->GetFlowState() == EDBALoginFlowState::ConnectingVillage)
		{
			if (!PlayerController->PlayerState)
			{
				UE_LOG(LogDBACore, Verbose, TEXT("[DBALobbyPlayerController] 大厅 PlayerState 尚未同步，暂不显示大厅界面。"));
				return;
			}
			LoginFlow->ConfirmVillageConnectionReady();
		}

		if (!UDBAGameInstance::CanEnterLobbyGameplay(GameInstance))
		{
			if (UDBAGameInstance* DBAInstance = Cast<UDBAGameInstance>(GameInstance))
			{
				UE_LOG(LogDBACore, Warning, TEXT("[DBALobbyPlayerController] 未完成登录/选角，返回前端流程。"));
				DBAInstance->StartLoginFlow();
			}
			return;
		}

		if (UDBAGameUIManager* UIManager = GameInstance ? GameInstance->GetSubsystem<UDBAGameUIManager>() : nullptr)
		{
			UIManager->ShowMainLobby();
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

}

void ADBALobbyPlayerController::BeginPlay()
{
	SuppressLobbyPawnCameraSteal(this);
	Super::BeginPlay();

	if (IsLocalController())
	{
		MouseYawSensitivityScale = FMath::Max(MouseYawSensitivityScale, 21.0f);
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		SetInputMode(InputMode);
		SetShowMouseCursor(true);
		EnsureCustomSoftwareCursor();
		bEnableClickEvents = true;
		bEnableTouchEvents = true;
		if (ShouldLockCameraToLobbyPawn(this))
		{
			if (APawn* ControlledPawn = GetPawn())
			{
				InitializeLobbyCameraForPawn(ControlledPawn);
				SetViewTarget(ControlledPawn);
				UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] BeginPlay 已将视角目标设置为角色：%s"),
					*ControlledPawn->GetName());
			}
		}
		else
		{
			bAutoManageActiveCameraTarget = false;
			RestorePresentationCameraIfNeeded(this);
			UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] BeginPlay 跳过锁定大厅角色相机（前端选角/创建流程）。"));
		}
		RequestLobbyHUDForLocalController(this);
		RequestConfiguredInputAssetsAsync();
	}
}

void ADBALobbyPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	RequestLobbyHUDForLocalController(this);
}

void ADBALobbyPlayerController::OnPossess(APawn* InPawn)
{
	SuppressLobbyPawnCameraSteal(this);
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
		if (ShouldLockCameraToLobbyPawn(this))
		{
			InitializeLobbyCameraForPawn(InPawn);
			SetViewTarget(InPawn);
			UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] OnPossess 已将视角目标设置为角色：%s"),
				*InPawn->GetName());
		}
		else
		{
			bAutoManageActiveCameraTarget = false;
			RestorePresentationCameraIfNeeded(this);
			UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] OnPossess 跳过锁定大厅角色相机（前端选角/创建流程）。Pawn=%s"),
				*InPawn->GetName());
		}
		RequestLobbyHUDForLocalController(this);
		RequestConfiguredInputAssetsAsync();
	}
}

void ADBALobbyPlayerController::AcknowledgePossession(APawn* P)
{
	SuppressLobbyPawnCameraSteal(this);
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
		if (ShouldLockCameraToLobbyPawn(this))
		{
			InitializeLobbyCameraForPawn(P);
			SetViewTarget(P);
			UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] AcknowledgePossession 已将视角目标设置为角色：%s"),
				*P->GetName());
		}
		else
		{
			bAutoManageActiveCameraTarget = false;
			RestorePresentationCameraIfNeeded(this);
			UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] AcknowledgePossession 跳过锁定大厅角色相机（前端选角/创建流程）。Pawn=%s"),
				*P->GetName());
		}
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

	RequestConfiguredInputAssetsAsync();
}

void ADBALobbyPlayerController::RequestConfiguredInputAssetsAsync()
{
	if (!IsLocalController() || !InputComponent)
	{
		return;
	}

	const UDBAEnhancedInputDeveloperSettings* Settings = GetDefault<UDBAEnhancedInputDeveloperSettings>();
	if (!Settings)
	{
		UE_LOG(LogDBACore, Error, TEXT("[大厅玩家控制器] 未找到 DBA Enhanced Input 设置，无法绑定输入。"));
		return;
	}

	const bool bArenaMode = IsArenaCombatInputMode();
	const TSoftObjectPtr<UDBALobbyInputConfigDataAsset> InputConfigRef = bArenaMode
		? Settings->DefaultArenaInputConfig
		: Settings->DefaultLobbyInputConfig;
	const TSoftObjectPtr<UInputMappingContext> MappingContextRef = bArenaMode
		? Settings->DefaultArenaInputMappingContext
		: Settings->DefaultLobbyInputMappingContext;
	if (InputConfigRef.IsNull() || MappingContextRef.IsNull())
	{
		UE_LOG(LogDBACore, Error, TEXT("[大厅玩家控制器] Enhanced Input 配置不完整，无法绑定输入：模式=%s。"), bArenaMode ? TEXT("竞技场") : TEXT("大厅"));
		return;
	}

	TWeakObjectPtr<ADBALobbyPlayerController> WeakThis(this);
	DBAAsyncAssetLoader::RequestAsyncAsset<UDBALobbyInputConfigDataAsset>(this, InputConfigRef, [WeakThis, MappingContextRef](UDBALobbyInputConfigDataAsset* LoadedConfig)
	{
		ADBALobbyPlayerController* StrongThis = WeakThis.Get();
		if (!StrongThis || !LoadedConfig)
		{
			if (StrongThis)
			{
				UE_LOG(LogDBACore, Error, TEXT("[大厅玩家控制器] 输入配置数据资产异步加载失败，无法绑定输入。"));
			}
			return;
		}

		DBAAsyncAssetLoader::RequestAsyncAsset<UInputMappingContext>(StrongThis, MappingContextRef, [WeakThis, WeakConfig = TWeakObjectPtr<UDBALobbyInputConfigDataAsset>(LoadedConfig)](UInputMappingContext* LoadedContext)
		{
			if (ADBALobbyPlayerController* BoundController = WeakThis.Get())
			{
				if (UDBALobbyInputConfigDataAsset* BoundConfig = WeakConfig.Get())
				{
					BoundController->BindConfiguredInput(BoundConfig, LoadedContext);
				}
			}
		});
	});
}

void ADBALobbyPlayerController::BindConfiguredInput(UDBALobbyInputConfigDataAsset* InputConfig, UInputMappingContext* MappingContext)
{
	UDBAEnhancedInputComponent* EnhancedInputComponent = Cast<UDBAEnhancedInputComponent>(InputComponent);
	if (!InputConfig || !MappingContext || !EnhancedInputComponent)
	{
		UE_LOG(LogDBACore, Error, TEXT("[大厅玩家控制器] Enhanced Input 资源加载完成但绑定条件不满足，无法绑定输入。"));
		return;
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (UInputMappingContext* PreviousContext = AppliedInputMappingContext.Get(); PreviousContext && PreviousContext != MappingContext)
			{
				InputSubsystem->RemoveMappingContext(PreviousContext);
			}
			InputSubsystem->AddMappingContext(MappingContext, IsArenaCombatInputMode() ? 1 : 0);
			AppliedInputMappingContext = MappingContext;
		}
	}

	if (bInputBindingsApplied)
	{
		return;
	}

	auto BindAxisAction = [&](const TSoftObjectPtr<UInputAction>& ActionRef, auto Callback)
	{
		if (UInputAction* Action = ActionRef.Get())
		{
			EnhancedInputComponent->BindAction(Action, ETriggerEvent::Triggered, this, Callback);
			EnhancedInputComponent->BindAction(Action, ETriggerEvent::Completed, this, Callback);
		}
	};

	BindAxisAction(InputConfig->MoveForward, &ADBALobbyPlayerController::OnMoveForward);
	BindAxisAction(InputConfig->MoveRight, &ADBALobbyPlayerController::OnMoveRight);
	BindAxisAction(InputConfig->Turn, &ADBALobbyPlayerController::OnTurn);
	BindAxisAction(InputConfig->LookUp, &ADBALobbyPlayerController::OnLookUp);

	auto BindBoolAction = [&](const TSoftObjectPtr<UInputAction>& ActionRef, auto Callback)
	{
		if (UInputAction* Action = ActionRef.Get())
		{
			EnhancedInputComponent->BindAction(Action, ETriggerEvent::Started, this, Callback);
		}
	};

	BindBoolAction(InputConfig->Skill01, &ADBALobbyPlayerController::OnSkill01);
	BindBoolAction(InputConfig->Skill02, &ADBALobbyPlayerController::OnSkill02);
	BindBoolAction(InputConfig->Skill03, &ADBALobbyPlayerController::OnSkill03);
	BindBoolAction(InputConfig->Skill04, &ADBALobbyPlayerController::OnSkill04);
	BindBoolAction(InputConfig->Ultimate, &ADBALobbyPlayerController::OnUltimate);
	BindBoolAction(InputConfig->Dodge, &ADBALobbyPlayerController::OnDodge);
	if (UInputAction* DodgeAction = InputConfig->Dodge.Get())
	{
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Completed, this, &ADBALobbyPlayerController::OnDodgeCompleted);
	}
	BindBoolAction(InputConfig->CycleLockTarget, &ADBALobbyPlayerController::OnCycleLockTarget);
	BindBoolAction(InputConfig->Skill06, &ADBALobbyPlayerController::OnSkill06);
	BindBoolAction(InputConfig->LeftMouse, &ADBALobbyPlayerController::OnLeftMouseStarted);
	if (UInputAction* LeftMouseAction = InputConfig->LeftMouse.Get())
	{
		EnhancedInputComponent->BindAction(LeftMouseAction, ETriggerEvent::Completed, this, &ADBALobbyPlayerController::OnLeftMouseCompleted);
	}
	BindBoolAction(InputConfig->RightMouse, &ADBALobbyPlayerController::OnRightMouseStarted);
	if (UInputAction* RightMouseAction = InputConfig->RightMouse.Get())
	{
		EnhancedInputComponent->BindAction(RightMouseAction, ETriggerEvent::Completed, this, &ADBALobbyPlayerController::OnRightMouseCompleted);
	}
	BindBoolAction(InputConfig->ScrollUp, &ADBALobbyPlayerController::OnScrollUp);
	BindBoolAction(InputConfig->ScrollDown, &ADBALobbyPlayerController::OnScrollDown);
	BindBoolAction(InputConfig->Inventory, &ADBALobbyPlayerController::OnInventory);
	BindBoolAction(InputConfig->Escape, &ADBALobbyPlayerController::OnEscape);

	bInputBindingsApplied = true;
	UE_LOG(LogDBACore, Log, TEXT("[大厅玩家控制器] Enhanced Input 异步加载并绑定完成：模式=%s。"), IsArenaCombatInputMode() ? TEXT("竞技场") : TEXT("大厅"));
}

void ADBALobbyPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (!IsLocalController())
	{
		return;
	}

	float ForwardValue = AnalogMoveForwardAxis;
	const float RightValue = AnalogMoveRightAxis;
	if (bLeftMouseLookHeld && bRightMouseLookHeld)
	{
		// 魔兽世界式镜头：左右键同时按住时，角色按当前镜头朝向持续前进。
		ForwardValue = FMath::Max(ForwardValue, 1.0f);
	}
	ApplyMovementInput(ForwardValue, RightValue);
	RefreshMouseLookCaptureMode();
	ApplyMouseLook();
	ApplyTouchLook();
}

void ADBALobbyPlayerController::OnMoveForward(const FInputActionValue& Value)
{
	const float AxisValue = Value.Get<float>();
	AnalogMoveForwardAxis = FMath::IsNearlyZero(AxisValue, 0.05f) ? 0.0f : FMath::Clamp(AxisValue, -1.0f, 1.0f);
}

void ADBALobbyPlayerController::OnMoveRight(const FInputActionValue& Value)
{
	const float AxisValue = Value.Get<float>();
	AnalogMoveRightAxis = FMath::IsNearlyZero(AxisValue, 0.05f) ? 0.0f : FMath::Clamp(AxisValue, -1.0f, 1.0f);
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

void ADBALobbyPlayerController::OnTurn(const FInputActionValue& Value)
{
	CachedTurnAxis = Value.Get<float>();
}

void ADBALobbyPlayerController::HandleLeftMousePressed()
{
	if (IsFrontendLoginWorld(GetWorld()) || IsCharacterPresentationFlowActive(this))
	{
		return;
	}

	SaveMouseLookCursorPosition();
	bLeftMouseLookHeld = true;
	bLeftMouseClickCandidate = true;
	LeftMouseDragDistance = 0.0f;
	RefreshMouseLookCaptureMode();
}

void ADBALobbyPlayerController::HandleLeftMouseReleased()
{
	if (IsFrontendLoginWorld(GetWorld()) || IsCharacterPresentationFlowActive(this))
	{
		return;
	}

	const bool bIsRightMouseDown = bRightMouseLookHeld;
	const bool bShouldSelectClickedTarget = bLeftMouseClickCandidate && !bIsRightMouseDown;
	bLeftMouseLookHeld = false;
	bLeftMouseClickCandidate = false;
	LeftMouseDragDistance = 0.0f;
	RefreshMouseLookCaptureMode();
	if (bShouldSelectClickedTarget)
	{
		if (IsArenaCombatInputMode())
		{
			HandleBasicAttackPressed();
		}
		else
		{
			HandleSelectTargetPressed();
		}
	}
}

void ADBALobbyPlayerController::HandleRightMousePressed()
{
	if (IsFrontendLoginWorld(GetWorld()) || IsCharacterPresentationFlowActive(this))
	{
		return;
	}

	SaveMouseLookCursorPosition();
	bRightMouseLookHeld = true;
	bLeftMouseClickCandidate = false;
	RefreshMouseLookCaptureMode();
}

void ADBALobbyPlayerController::HandleRightMouseReleased()
{
	if (IsFrontendLoginWorld(GetWorld()) || IsCharacterPresentationFlowActive(this))
	{
		return;
	}

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

// ==================== P1-4 Enhanced Input Action 回调 ====================

void ADBALobbyPlayerController::OnSkill01(const FInputActionValue& Value)
{
	HandleSkill01Pressed();
}

void ADBALobbyPlayerController::OnSkill02(const FInputActionValue& Value)
{
	HandleSkill02Pressed();
}

void ADBALobbyPlayerController::OnSkill03(const FInputActionValue& Value)
{
	HandleSkill03Pressed();
}

void ADBALobbyPlayerController::OnSkill04(const FInputActionValue& Value)
{
	HandleSkill04Pressed();
}

void ADBALobbyPlayerController::OnUltimate(const FInputActionValue& Value)
{
	HandleUltimatePressed();
}

void ADBALobbyPlayerController::OnSkill06(const FInputActionValue& Value)
{
	HandleSkill06Pressed();
}

void ADBALobbyPlayerController::OnBasicAttack(const FInputActionValue& Value)
{
	HandleBasicAttackPressed();
}

void ADBALobbyPlayerController::OnDodge(const FInputActionValue& Value)
{
	if (IsArenaCombatInputMode())
	{
		HandleDodgePressed();
		return;
	}

	if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
	{
		ControlledCharacter->Jump();
		bJumpHeld = true;
	}
}

void ADBALobbyPlayerController::OnDodgeCompleted(const FInputActionValue& Value)
{
	if (!IsArenaCombatInputMode())
	{
		if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
		{
			ControlledCharacter->StopJumping();
		}
		bJumpHeld = false;
	}
}

void ADBALobbyPlayerController::OnLeftMouseStarted(const FInputActionValue& Value)
{
	HandleLeftMousePressed();
}

void ADBALobbyPlayerController::OnLeftMouseCompleted(const FInputActionValue& Value)
{
	HandleLeftMouseReleased();
}

void ADBALobbyPlayerController::OnRightMouseStarted(const FInputActionValue& Value)
{
	HandleRightMousePressed();
}

void ADBALobbyPlayerController::OnRightMouseCompleted(const FInputActionValue& Value)
{
	HandleRightMouseReleased();
}

void ADBALobbyPlayerController::OnScrollUp(const FInputActionValue& Value)
{
	HandleMouseScrollUp();
}

void ADBALobbyPlayerController::OnScrollDown(const FInputActionValue& Value)
{
	HandleMouseScrollDown();
}

void ADBALobbyPlayerController::OnInventory(const FInputActionValue& Value)
{
	HandleInventoryPressed();
}

void ADBALobbyPlayerController::OnEscape(const FInputActionValue& Value)
{
	HandleEscapePressed();
}

void ADBALobbyPlayerController::OnCycleLockTarget(const FInputActionValue& Value)
{
	HandleCycleLockTargetPressed();
}

void ADBALobbyPlayerController::RequestBasicAttack()
{
	HandleBasicAttackPressed();
}

void ADBALobbyPlayerController::RequestDodge()
{
	HandleDodgePressed();
}

void ADBALobbyPlayerController::CycleLockTarget()
{
	HandleCycleLockTargetPressed();
}

void ADBALobbyPlayerController::HandleBasicAttackPressed()
{
	if (!IsLocalController())
	{
		return;
	}

	ADBAZodiacCharacterBase* ZodiacPawn = Cast<ADBAZodiacCharacterBase>(GetPawn());
	if (!ZodiacPawn)
	{
		return;
	}

	AActor* Target = SelectedAttackTarget.Get();
	if (ADBALobbyTrainingMonster* TrainingTarget = Cast<ADBALobbyTrainingMonster>(Target))
	{
		if (TrainingTarget->GetHealthPercent() <= 0.0f)
		{
			Target = nullptr;
		}
	}
	if (!Target)
	{
		Target = ResolveAutoAttackTarget();
	}

	if (UDBAAbilitySystemComponent* ASC = ZodiacPawn->GetDBAAbilitySystemComponent())
	{
		if (ASC->TryActivateAbilityByInputID(static_cast<int32>(EDBAAbilityInputID::BasicAttack), Target))
		{
			UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] 已通过 GAS 触发普攻：角色=%s 目标=%s"),
				*ZodiacPawn->GetName(),
				*GetNameSafe(Target));
			return;
		}
	}

	if (ADBARpcHandler* RpcHandler = ZodiacPawn->GetRpcHandler())
	{
		RpcHandler->ServerRequestAttack();
		UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] 已通过 RPC 请求普攻：角色=%s 目标=%s"),
			*ZodiacPawn->GetName(),
			*GetNameSafe(Target));
		return;
	}

	UE_LOG(LogDBACore, Warning, TEXT("[DBALobbyPlayerController] 普攻失败：GAS 与 RpcHandler 均未就绪。"));
}

void ADBALobbyPlayerController::HandleDodgePressed()
{
	if (!IsLocalController())
	{
		return;
	}

	ADBAZodiacCharacterBase* ZodiacPawn = Cast<ADBAZodiacCharacterBase>(GetPawn());
	if (!ZodiacPawn)
	{
		return;
	}

	// 预留 GAS Dodge 能力接入点：当前尚无 EDBAAbilityInputID::Dodge，仅记录中文日志
	UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] 收到闪避输入：角色=%s（GAS Dodge 能力尚未接入，待后续 Ability + InputID 绑定）"),
		*ZodiacPawn->GetName());
}

void ADBALobbyPlayerController::HandleCycleLockTargetPressed()
{
	if (!IsLocalController())
	{
		return;
	}

	ADBAZodiacCharacterBase* ZodiacPawn = Cast<ADBAZodiacCharacterBase>(GetPawn());
	if (!ZodiacPawn)
	{
		return;
	}

	const TArray<AActor*> Candidates = CollectLockTargetCandidates();
	if (Candidates.Num() == 0)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBALobbyPlayerController] 切换锁定目标失败：范围内无可锁定敌对单位。"));
		return;
	}

	ADBARpcHandler* RpcHandler = ZodiacPawn->GetRpcHandler();
	AActor* CurrentLockedTarget = RpcHandler ? RpcHandler->GetLockedTargetActor() : SelectedAttackTarget.Get();
	int32 NextIndex = 0;
	if (CurrentLockedTarget)
	{
		const int32 CurrentIndex = Candidates.IndexOfByKey(CurrentLockedTarget);
		if (CurrentIndex != INDEX_NONE)
		{
			NextIndex = (CurrentIndex + 1) % Candidates.Num();
		}
	}

	AActor* NextTarget = Candidates[NextIndex];
	if (RpcHandler)
	{
		RpcHandler->ServerLockTarget(NextTarget);
	}

	if (ADBALobbyTrainingMonster* PreviousTarget = Cast<ADBALobbyTrainingMonster>(SelectedAttackTarget.Get()))
	{
		PreviousTarget->SetLobbySelected(false);
	}
	SelectedAttackTarget = NextTarget;
	if (ADBALobbyTrainingMonster* NewTarget = Cast<ADBALobbyTrainingMonster>(NextTarget))
	{
		NewTarget->SetLobbySelected(true);
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBALobbyPlayerController] 已切换锁定目标：%s（候选数=%d）"),
		*GetNameSafe(NextTarget),
		Candidates.Num());
}

bool ADBALobbyPlayerController::IsArenaCombatInputMode() const
{
	return !IsLobbyGameplayWorld(GetWorld());
}

TArray<AActor*> ADBALobbyPlayerController::CollectLockTargetCandidates() const
{
	TArray<AActor*> Candidates;
	const APawn* ControlledPawn = GetPawn();
	const UWorld* World = GetWorld();
	if (!ControlledPawn || !World)
	{
		return Candidates;
	}

	const FVector Origin = ControlledPawn->GetActorLocation();
	const FRotator ControlRot = GetControlRotation();
	const FVector Forward = ControlRot.Vector().GetSafeNormal2D();
	constexpr float MaxLockTargetDistance = 1800.0f;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DBA_CycleLockTarget), false, ControlledPawn);
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(MaxLockTargetDistance);

	World->OverlapMultiByObjectType(
		Overlaps,
		Origin,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		SphereShape,
		QueryParams);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!IsClientSideEnemyCandidate(ControlledPawn, Candidate))
		{
			continue;
		}

		if (const ADBALobbyTrainingMonster* TrainingMonster = Cast<ADBALobbyTrainingMonster>(Candidate))
		{
			if (TrainingMonster->GetHealthPercent() <= 0.0f)
			{
				continue;
			}
		}

		Candidates.AddUnique(Candidate);
	}

	Candidates.Sort([Origin, Forward](const AActor& A, const AActor& B)
	{
		const FVector DirA = (A.GetActorLocation() - Origin).GetSafeNormal2D();
		const FVector DirB = (B.GetActorLocation() - Origin).GetSafeNormal2D();
		const float AngleA = FMath::Atan2(
			FVector::CrossProduct(Forward, DirA).Z,
			FVector::DotProduct(Forward, DirA));
		const float AngleB = FMath::Atan2(
			FVector::CrossProduct(Forward, DirB).Z,
			FVector::DotProduct(Forward, DirB));
		return AngleA < AngleB;
	});

	return Candidates;
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

void ADBALobbyPlayerController::OnLookUp(const FInputActionValue& Value)
{
	CachedLookUpAxis = Value.Get<float>();
}

void ADBALobbyPlayerController::ApplyMovementInput(float ForwardValue, float RightValue)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	const FRotator ControlRot = GetControlRotation();
	const bool bUseMouseFacing = bRightMouseLookHeld;
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

void ADBALobbyPlayerController::ApplyMouseLook()
{
	const bool bIsLeftLooking = bLeftMouseLookHeld;
	const bool bIsRightLooking = bRightMouseLookHeld;
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
	if (IsFrontendLoginWorld(GetWorld()) || IsCharacterPresentationFlowActive(this))
	{
		// 前端登录、选角和创建角色由 UMG 接管鼠标输入，禁止大厅镜头逻辑抢占点击和键盘焦点。
		bLeftMouseLookHeld = false;
		bRightMouseLookHeld = false;
		bLeftMouseClickCandidate = false;
		LeftMouseDragDistance = 0.0f;
		bMouseLookCaptureActive = false;
		return;
	}

	const bool bShouldCapture = bLeftMouseLookHeld || bRightMouseLookHeld;
	SetMouseLookCaptureActive(bShouldCapture);
	ConfigurePawnForRightMouseLook(bRightMouseLookHeld);
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
