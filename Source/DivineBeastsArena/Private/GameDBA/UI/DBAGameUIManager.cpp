// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/DBAGameUIManager.h"
#include "GameDBA/UI/Splash/UDBASplashVideoWidget.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GameInstance/DBAGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendTelemetryService.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameDBA/UI/Arena/UDBAArenaHUDRootWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBALobbyPlayerHUDWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.h"
#include "GameDBA/UI/Lobby/Loading/UDBALoadingScreenWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAGameSettingsWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAInventoryWidgetBase.h"
#include "GameDBA/UI/Common/UDBASoftwareCursorWidget.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	FString BuildBlueprintClassObjectPath(const TCHAR* CandidatePath)
	{
		if (!CandidatePath || !(*CandidatePath))
		{
			return FString();
		}

		const FString RawPath(CandidatePath);
		if (RawPath.EndsWith(TEXT("_C")))
		{
			return RawPath;
		}

		const FString PackageName = RawPath.Contains(TEXT("."))
			? FPackageName::ObjectPathToPackageName(RawPath)
			: RawPath;
		const FString AssetName = FPackageName::GetLongPackageAssetName(PackageName);
		if (PackageName.IsEmpty() || AssetName.IsEmpty())
		{
			return FString();
		}

		return FString::Printf(TEXT("%s.%s_C"), *PackageName, *AssetName);
	}

	template<typename WidgetType>
	TSubclassOf<WidgetType> ResolveWidgetClassPath(std::initializer_list<const TCHAR*> CandidatePaths)
	{
		for (const TCHAR* CandidatePath : CandidatePaths)
		{
			const FString ClassObjectPath = BuildBlueprintClassObjectPath(CandidatePath);
			if (ClassObjectPath.IsEmpty())
			{
				continue;
			}

			const FSoftClassPath SoftClassPath(ClassObjectPath);
			if (UClass* LoadedClass = SoftClassPath.TryLoadClass<WidgetType>())
			{
				return LoadedClass;
			}
		}
		return nullptr;
	}

	void ApplyFrontendInputMode(UWorld* World, UUserWidget* FocusWidget)
	{
		if (!World || !FocusWidget)
		{
			return;
		}

		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC)
		{
			return;
		}

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->CurrentMouseCursor = EMouseCursor::Default;
		PC->DefaultMouseCursor = EMouseCursor::Default;
		PC->SetShowMouseCursor(true);
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		if (UDBASoftwareCursorWidget* CursorWidget = CreateWidget<UDBASoftwareCursorWidget>(PC, UDBASoftwareCursorWidget::StaticClass()))
		{
			PC->SetMouseCursorWidget(EMouseCursor::Default, CursorWidget);
			PC->SetMouseCursorWidget(EMouseCursor::Crosshairs, CursorWidget);
			PC->SetMouseCursorWidget(EMouseCursor::Hand, CursorWidget);
		}
		FocusWidget->SetFocus();
	}

	template<typename AssetType>
	AssetType* LoadAssetIfCookedAvailable_UIManager(const TCHAR* ObjectPath)
	{
		if (!ObjectPath)
		{
			return nullptr;
		}

		const FSoftObjectPath SoftPath(ObjectPath);
		const FString PackageName = SoftPath.GetLongPackageName();
		if (PackageName.IsEmpty() || !FPackageName::DoesPackageExist(PackageName))
		{
			return nullptr;
		}

		return Cast<AssetType>(SoftPath.TryLoad());
	}

	void ApplySplashInputMode(UWorld* World, UUserWidget* FocusWidget)
	{
		if (!World || !FocusWidget)
		{
			return;
		}

		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC)
		{
			return;
		}

		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->CurrentMouseCursor = EMouseCursor::Default;
		PC->DefaultMouseCursor = EMouseCursor::Default;
		PC->SetShowMouseCursor(true);
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		if (UDBASoftwareCursorWidget* CursorWidget = CreateWidget<UDBASoftwareCursorWidget>(PC, UDBASoftwareCursorWidget::StaticClass()))
		{
			PC->SetMouseCursorWidget(EMouseCursor::Default, CursorWidget);
			PC->SetMouseCursorWidget(EMouseCursor::Crosshairs, CursorWidget);
			PC->SetMouseCursorWidget(EMouseCursor::Hand, CursorWidget);
		}
		FocusWidget->SetFocus();
	}

	void RemoveAllViewportLoginWidgets(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, UDBALoginFlowWidgetBase::StaticClass(), false);
		for (UUserWidget* Widget : FoundWidgets)
		{
			if (Widget && Widget->IsInViewport())
			{
				Widget->RemoveFromParent();
			}
		}
	}

	void ApplyLobbyGameplayInputMode(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC)
		{
			return;
		}

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->CurrentMouseCursor = EMouseCursor::Default;
		PC->DefaultMouseCursor = EMouseCursor::Default;
		PC->SetShowMouseCursor(true);
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = false;
		if (UDBASoftwareCursorWidget* CursorWidget = CreateWidget<UDBASoftwareCursorWidget>(PC, UDBASoftwareCursorWidget::StaticClass()))
		{
			PC->SetMouseCursorWidget(EMouseCursor::Default, CursorWidget);
			PC->SetMouseCursorWidget(EMouseCursor::Crosshairs, CursorWidget);
			PC->SetMouseCursorWidget(EMouseCursor::Hand, CursorWidget);
		}
	}

	bool IsServerLikeCommandLine()
	{
		return IsRunningDedicatedServer()
			|| FParse::Param(FCommandLine::Get(), TEXT("server"))
			|| FParse::Param(FCommandLine::Get(), TEXT("DBAHeadlessLobbyServer"));
	}

	bool IsServerLikeRuntime(const UWorld* World)
	{
		return IsServerLikeCommandLine()
			|| (World && World->GetNetMode() == NM_DedicatedServer);
	}

	bool IsLobbyGameplayWorldForUIManager(const UWorld* World)
	{
		if (!World || !World->PersistentLevel)
		{
			return false;
		}

		const FString LevelPath = World->PersistentLevel->GetOutermost()->GetName();
		return LevelPath.Contains(TEXT("LobbyMap")) || LevelPath.Contains(TEXT("MainLobby"));
	}

	void CenterModalWidgetInViewport(UUserWidget* Widget, const FVector2D& DesiredSize)
	{
		if (!Widget)
		{
			return;
		}

		Widget->SetAnchorsInViewport(FAnchors(0.5f, 0.5f));
		Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
		Widget->SetPositionInViewport(FVector2D::ZeroVector, false);
		Widget->SetDesiredSizeInViewport(DesiredSize);
	}
}

UDBAGameUIManager::UDBAGameUIManager()
	: Super()
{
	if (IsServerLikeCommandLine())
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 检测到服务器运行环境，构造阶段跳过 UI 类加载。"));
		return;
	}

	MainLobbyWidgetClass = ResolveWidgetClassPath<UDBAMainLobbyWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/MainLobby/WBP_DBA_MainLobby"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_MainLobby")
	});
	if (!MainLobbyWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 主大厅控件蓝图不可用。"));
	}

	ArenaHUDWidgetClass = ResolveWidgetClassPath<UDBAArenaHUDRootWidgetBase>({
		TEXT("/Game/UI/Arena/HUD/WBP_DBA_ArenaHUDRoot"),
		TEXT("/Game/Blueprints/UI/DBA/ArenaHUD/WBP_DBA_ArenaHUDRoot")
	});
	if (!ArenaHUDWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 竞技场 HUD 控件蓝图不可用。"));
	}

	LoginWidgetClass = ResolveWidgetClassPath<UDBALoginFlowWidgetBase>({
		TEXT("/Game/DBA/UI/Frontend/Login/WBP_DBA_Login")
	});
	if (!LoginWidgetClass)
	{
		LoginWidgetClass = UDBALoginFlowWidgetBase::StaticClass();
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 登录控件蓝图不可用，使用 C++ 原生兜底控件。"));
	}

	CharacterSelectWidgetClass = ResolveWidgetClassPath<UDBACharacterSelectFlowWidgetBase>({
		TEXT("/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterSelect"),
		TEXT("/Game/DBA/UI/Lobby/Character/WBP_DBA_CharacterSelect"),
		TEXT("/Game/Blueprints/UI/DBA/Login/WBP_DBA_CharacterSelect")
	});
	if (!CharacterSelectWidgetClass)
	{
		CharacterSelectWidgetClass = UDBACharacterSelectFlowWidgetBase::StaticClass();
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 角色选择控件蓝图不可用，使用 C++ 原生兜底控件。"));
	}

	CharacterCreateWidgetClass = ResolveWidgetClassPath<UDBACharacterCreateFlowWidgetBase>({
		TEXT("/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterCreate"),
		TEXT("/Game/DBA/UI/Lobby/Character/WBP_DBA_CharacterCreate"),
		TEXT("/Game/Blueprints/UI/DBA/Login/WBP_DBA_CharacterCreate")
	});
	if (!CharacterCreateWidgetClass)
	{
		CharacterCreateWidgetClass = UDBACharacterCreateFlowWidgetBase::StaticClass();
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 角色创建控件蓝图不可用，使用 C++ 原生兜底控件。"));
	}

	LobbyLoadingWidgetClass = ResolveWidgetClassPath<UDBALoadingScreenWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/Loading/WBP_DBA_LoadingScreen"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_LoadingScreen")
	});
	if (!LobbyLoadingWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 大厅加载控件蓝图不可用。"));
	}

	LobbyPlayerHUDWidgetClass = ResolveWidgetClassPath<UDBALobbyPlayerHUDWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/HUD/WBP_DBA_LobbyPlayerHUD"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_LobbyPlayerHUD")
	});
	if (!LobbyPlayerHUDWidgetClass)
	{
		LobbyPlayerHUDWidgetClass = UDBALobbyPlayerHUDWidgetBase::StaticClass();
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 大厅玩家 HUD 控件蓝图不可用，使用 C++ 原生兜底控件。"));
	}

	GameSettingsWidgetClass = ResolveWidgetClassPath<UDBAGameSettingsWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/System/WBP_DBA_GameSettings"),
		TEXT("/Game/DBA/UI/Lobby/Settings/WBP_DBA_GameSettings"),
		TEXT("/Game/DBA/UI/Lobby/Settings/WBP_DBA_SettingsRoot"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_GameSettings")
	});
	if (!GameSettingsWidgetClass)
	{
		GameSettingsWidgetClass = UDBAGameSettingsWidgetBase::StaticClass();
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 游戏设置控件蓝图不可用，使用 C++ 原生兜底控件。"));
	}

	InventoryWidgetClass = ResolveWidgetClassPath<UDBAInventoryWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/Inventory/WBP_DBA_Inventory"),
		TEXT("/Game/DBA/UI/Lobby/Bag/WBP_DBA_Inventory"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_Inventory")
	});
	if (!InventoryWidgetClass)
	{
		InventoryWidgetClass = UDBAInventoryWidgetBase::StaticClass();
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 背包控件蓝图不可用，使用 C++ 原生兜底控件。"));
	}

	static ConstructorHelpers::FClassFinder<UDBASplashVideoWidget> SplashVideoWidgetFinder(TEXT("/Game/UI/Splash/WBP_DBA_SplashVideo"));
	if (SplashVideoWidgetFinder.Succeeded())
	{
		SplashVideoWidgetClass = SplashVideoWidgetFinder.Class;
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 已找到启动视频控件类：%s"), *SplashVideoWidgetClass->GetName());
	}
	else
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 未找到启动视频控件类。"));
	}
}

void UDBAGameUIManager::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();

	if (IsServerLikeRuntime(GetWorld()))
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 服务器运行环境跳过前端 UI 初始化。"));
		return;
	}
	if (FParse::Param(FCommandLine::Get(), TEXT("DBASkipSplash")))
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 命令行要求跳过启动视频。"));
		EnsureLoginFlowStartedFromManager();
		return;
	}

	if (UDBALoginFlowSubsystem* LoginFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>() : nullptr)
	{
		LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBAGameUIManager::HandleLoginFlowStateChanged);
		LoginFlow->OnFlowStateChanged.AddDynamic(this, &UDBAGameUIManager::HandleLoginFlowStateChanged);
		CachedLoginFlowState = LoginFlow->GetFlowState();
		RefreshLoginFlowWidgetVisibility();
	}

	// 延迟显示启动视频，等待世界加载完成
	GetWorld()->GetTimerManager().SetTimer(SplashVideoTimerHandle, this, &UDBAGameUIManager::TryShowSplashVideo, 0.5f, true);
}

void UDBAGameUIManager::TryShowSplashVideo()
{
	UWorld* World = GetWorld();
	if (IsServerLikeRuntime(World))
	{
		if (World)
		{
			World->GetTimerManager().ClearTimer(SplashVideoTimerHandle);
		}
		return;
	}
	if (!World)
	{
		EnsureLoginFlowStartedFromManager();
		return;
	}

	if (!SplashVideoWidgetClass)
	{
		GetWorld()->GetTimerManager().ClearTimer(SplashVideoTimerHandle);
		EnsureLoginFlowStartedFromManager();
		return;
	}

	// 先尝试获取 PrimaryPlayerController
	APlayerController* PC = World->GetGameInstance() ? World->GetGameInstance()->GetPrimaryPlayerController() : nullptr;
	if (!PC)
	{
		PC = World->GetFirstPlayerController();
	}

	if (PC)
	{
		// PlayerController 已存在，停止重试并显示视频
		GetWorld()->GetTimerManager().ClearTimer(SplashVideoTimerHandle);
		ShowSplashVideo();
	}
	else
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 正在等待 PlayerController，世界=%s"), *World->GetName());
	}
}

void UDBAGameUIManager::EnsureLoginFlowStartedFromManager()
{
	if (bLoginFlowStartRequested)
	{
		return;
	}

	if (UDBAGameInstance* DBAInstance = GetGameInstance() ? Cast<UDBAGameInstance>(GetGameInstance()) : nullptr)
	{
		DBAInstance->StartLoginFlow();
		bLoginFlowStartRequested = true;
		return;
	}

	if (UDBALoginFlowSubsystem* LoginFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>() : nullptr)
	{
		if (LoginFlow->GetFlowState() == EDBALoginFlowState::Startup)
		{
			LoginFlow->StartLoginFlow();
		}
		bLoginFlowStartRequested = true;
	}
}

void UDBAGameUIManager::OnSubsystemDeinitialize()
{
	if (UDBALoginFlowSubsystem* LoginFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>() : nullptr)
	{
		LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBAGameUIManager::HandleLoginFlowStateChanged);
	}

	StopLoginFlowBackgroundMusic();
	ClearAllUI();
	Super::OnSubsystemDeinitialize();
}

void UDBAGameUIManager::TransitionTo(EDBAUIState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const bool bReturningToLobbyFromMatch = CurrentState == EDBAUIState::InGame && NewState == EDBAUIState::Lobby;
	if (bReturningToLobbyFromMatch)
	{
		if (UGameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGameBackendClientSubsystem>() : nullptr)
		{
			if (Backend->GetTelemetryService())
			{
				Backend->GetTelemetryService()->TrackEvent(TEXT("match_finished_client_view"), TMap<FString, FString>());
			}
		}
	}

	switch (CurrentState)
	{
	case EDBAUIState::MainMenu:
	case EDBAUIState::Lobby:
		HideMainLobby();
		break;
	case EDBAUIState::InGame:
		HideArenaHUD();
		break;
	default:
		break;
	}

	CurrentState = NewState;
	OnStateChanged.Broadcast(NewState);

	switch (NewState)
	{
	case EDBAUIState::MainMenu:
	case EDBAUIState::Lobby:
		ShowMainLobby();
		break;
	case EDBAUIState::InGame:
		ShowArenaHUD();
		break;
	default:
		break;
	}
}

void UDBAGameUIManager::RegisterStateChangeCallback(const FOnUIStateChanged& Delegate)
{
	(void)Delegate;
}

void UDBAGameUIManager::ShowMainLobby()
{
	StopLoginFlowBackgroundMusic();
	HideAllFlowWidgets();

	if (IsLobbyGameplayWorldForUIManager(GetWorld()))
	{
		HideLobbyLoadingScreen();
		if (MainLobbyWidget && bMainLobbyVisible)
		{
			MainLobbyWidget->RemoveFromParent();
			bMainLobbyVisible = false;
		}

		ShowLobbyPlayerHUD();
		ApplyLobbyGameplayInputMode(GetWorld());
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 已为大厅地图显示游戏 HUD。"));
		return;
	}

	if (MainLobbyWidget)
	{
		const UWorld* CurrentWorld = GetWorld();
		const UWorld* WidgetWorld = MainLobbyWidget->GetWorld();
		const APlayerController* OwningPC = MainLobbyWidget->GetOwningPlayer();
		const UWorld* OwningPlayerWorld = OwningPC ? OwningPC->GetWorld() : nullptr;
		const bool bWidgetWorldMatches = CurrentWorld && (WidgetWorld == CurrentWorld || OwningPlayerWorld == CurrentWorld);
		if (!bWidgetWorldMatches)
		{
			MainLobbyWidget->RemoveFromParent();
			MainLobbyWidget = nullptr;
			bMainLobbyVisible = false;
		}
	}

	if (!MainLobbyWidget)
	{
		CreateMainLobbyWidget();
	}
	if (MainLobbyWidget && !bMainLobbyVisible)
	{
		HideLobbyLoadingScreen();
		MainLobbyWidget->AddToViewport(0);
		bMainLobbyVisible = true;
	}

	ShowLobbyPlayerHUD();
	ApplyLobbyGameplayInputMode(GetWorld());
}

void UDBAGameUIManager::HideMainLobby()
{
	if (MainLobbyWidget && bMainLobbyVisible)
	{
		MainLobbyWidget->RemoveFromParent();
		bMainLobbyVisible = false;
	}
	HideLobbyPlayerHUD();
}

void UDBAGameUIManager::RequestShowLoginFlowWidget()
{
	if (UDBALoginFlowSubsystem* LoginFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>() : nullptr)
	{
		CachedLoginFlowState = LoginFlow->GetFlowState();
	}
	ShowLoginFlowWidget();
}

void UDBAGameUIManager::ShowLoginFlowWidget()
{
	RefreshLoginFlowWidgetVisibility();
}

void UDBAGameUIManager::HideLoginFlowWidget()
{
	HideAllFlowWidgets();
}

void UDBAGameUIManager::ShowArenaHUD()
{
	if (!ArenaHUDWidget)
	{
		CreateArenaHUDWidget();
	}
	if (ArenaHUDWidget && !bArenaHUDVisible)
	{
		ArenaHUDWidget->AddToViewport(100);
		bArenaHUDVisible = true;
	}
}

void UDBAGameUIManager::HideArenaHUD()
{
	if (ArenaHUDWidget && bArenaHUDVisible)
	{
		ArenaHUDWidget->RemoveFromParent();
		bArenaHUDVisible = false;
	}
}

void UDBAGameUIManager::ClearAllUI()
{
	StopLoginFlowBackgroundMusic();
	HideLobbyLoadingScreen();
	HideAllFlowWidgets();
	HideMainLobby();
	HideLobbyPlayerHUD();
	HideArenaHUD();
	HideGameSettings();
	HideInventory();
}

void UDBAGameUIManager::ShowLobbyLoadingScreen()
{
	UWorld* World = GetWorld();
	if (!World || IsServerLikeRuntime(World))
	{
		return;
	}

	if (LobbyLoadingWidget)
	{
		const UWorld* WidgetWorld = LobbyLoadingWidget->GetWorld();
		const APlayerController* OwningPC = LobbyLoadingWidget->GetOwningPlayer();
		const UWorld* OwningPlayerWorld = OwningPC ? OwningPC->GetWorld() : nullptr;
		const bool bWidgetWorldMatches = WidgetWorld == World || OwningPlayerWorld == World;
		if (!bWidgetWorldMatches)
		{
			LobbyLoadingWidget->RemoveFromParent();
			LobbyLoadingWidget = nullptr;
			bLobbyLoadingVisible = false;
		}
	}

	if (!LobbyLoadingWidget && LobbyLoadingWidgetClass)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC && World->GetGameInstance())
		{
			PC = World->GetGameInstance()->GetPrimaryPlayerController();
		}
		if (PC)
		{
			LobbyLoadingWidget = CreateWidget<UDBALoadingScreenWidgetBase>(PC, LobbyLoadingWidgetClass);
		}
	}

	if (LobbyLoadingWidget && !bLobbyLoadingVisible)
	{
		HideMainLobby();
		HideAllFlowWidgets();
		LobbyLoadingWidget->AddToViewport(10000);
		LobbyLoadingWidget->UpdateLoadingProgress(0.35f);
		LobbyLoadingWidget->ShowTips(NSLOCTEXT("DBAGameUIManager", "EnteringLobby", "Entering lobby..."));
		ApplyFrontendInputMode(World, LobbyLoadingWidget);
		bLobbyLoadingVisible = true;
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 已显示大厅加载界面。"));
	}
}

void UDBAGameUIManager::HideLobbyLoadingScreen()
{
	if (LobbyLoadingWidget && bLobbyLoadingVisible)
	{
		LobbyLoadingWidget->UpdateLoadingProgress(1.0f);
		LobbyLoadingWidget->RemoveFromParent();
	}
	bLobbyLoadingVisible = false;
}

void UDBAGameUIManager::ShowGameSettings()
{
	UWorld* World = GetWorld();
	if (!World || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!GameSettingsWidget)
	{
		CreateGameSettingsWidget();
	}
	if (GameSettingsWidget && !bGameSettingsVisible)
	{
		GameSettingsWidget->AddToViewport(12000);
		CenterModalWidgetInViewport(GameSettingsWidget, FVector2D(680.0f, 520.0f));
		GameSettingsWidget->RefreshFromRuntime();
		ApplyFrontendInputMode(World, GameSettingsWidget);
		bGameSettingsVisible = true;
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 已显示游戏设置界面。"));
	}
}

void UDBAGameUIManager::HideGameSettings()
{
	if (GameSettingsWidget && bGameSettingsVisible)
	{
		GameSettingsWidget->RemoveFromParent();
	}
	bGameSettingsVisible = false;

	if (InventoryWidget && bInventoryVisible)
	{
		ApplyFrontendInputMode(GetWorld(), InventoryWidget);
	}
	else
	{
		ApplyLobbyGameplayInputMode(GetWorld());
	}
}

void UDBAGameUIManager::ToggleGameSettings()
{
	if (bGameSettingsVisible)
	{
		HideGameSettings();
	}
	else
	{
		ShowGameSettings();
	}
}

void UDBAGameUIManager::ShowInventory()
{
	UWorld* World = GetWorld();
	if (!World || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!InventoryWidget)
	{
		CreateInventoryWidget();
	}
	if (InventoryWidget && !bInventoryVisible)
	{
		InventoryWidget->AddToViewport(11000);
		CenterModalWidgetInViewport(InventoryWidget, FVector2D(760.0f, 540.0f));
		InventoryWidget->RefreshInventory();
		ApplyFrontendInputMode(World, InventoryWidget);
		bInventoryVisible = true;
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 已显示背包界面。"));
	}
}

void UDBAGameUIManager::HideInventory()
{
	if (InventoryWidget && bInventoryVisible)
	{
		InventoryWidget->RemoveFromParent();
	}
	bInventoryVisible = false;

	if (GameSettingsWidget && bGameSettingsVisible)
	{
		ApplyFrontendInputMode(GetWorld(), GameSettingsWidget);
	}
	else
	{
		ApplyLobbyGameplayInputMode(GetWorld());
	}
}

void UDBAGameUIManager::ToggleInventory()
{
	if (bInventoryVisible)
	{
		HideInventory();
	}
	else
	{
		ShowInventory();
	}
}

void UDBAGameUIManager::ShowLobbyPlayerHUD()
{
	if (LobbyPlayerHUDWidget)
	{
		const UWorld* CurrentWorld = GetWorld();
		const UWorld* WidgetWorld = LobbyPlayerHUDWidget->GetWorld();
		const APlayerController* OwningPC = LobbyPlayerHUDWidget->GetOwningPlayer();
		const UWorld* OwningPlayerWorld = OwningPC ? OwningPC->GetWorld() : nullptr;
		const bool bWidgetWorldMatches = CurrentWorld && (WidgetWorld == CurrentWorld || OwningPlayerWorld == CurrentWorld);
		if (!bWidgetWorldMatches)
		{
			LobbyPlayerHUDWidget->RemoveFromParent();
			LobbyPlayerHUDWidget = nullptr;
			bLobbyPlayerHUDVisible = false;
		}
	}

	if (!LobbyPlayerHUDWidget)
	{
		CreateLobbyPlayerHUDWidget();
	}

	if (LobbyPlayerHUDWidget && !bLobbyPlayerHUDVisible)
	{
		LobbyPlayerHUDWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		LobbyPlayerHUDWidget->AddToViewport(1000);
		LobbyPlayerHUDWidget->RefreshFromCurrentCharacterData();
		bLobbyPlayerHUDVisible = true;
		ResetLobbyHUDRefreshRetry();
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 大厅玩家 HUD 已添加到视口：%s"),
			*LobbyPlayerHUDWidget->GetClass()->GetName());
	}
	else if (LobbyPlayerHUDWidget && bLobbyPlayerHUDVisible)
	{
		LobbyPlayerHUDWidget->RefreshFromCurrentCharacterData();
	}
	else if (!LobbyPlayerHUDWidget && IsLobbyGameplayWorldForUIManager(GetWorld()))
	{
		ScheduleLobbyHUDRefreshRetry();
	}
}

void UDBAGameUIManager::HideLobbyPlayerHUD()
{
	if (LobbyPlayerHUDWidget && bLobbyPlayerHUDVisible)
	{
		LobbyPlayerHUDWidget->RemoveFromParent();
		bLobbyPlayerHUDVisible = false;
	}
}

void UDBAGameUIManager::CreateMainLobbyWidget()
{
	if (!MainLobbyWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			MainLobbyWidget = CreateWidget<UDBAMainLobbyWidgetBase>(PC, MainLobbyWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateArenaHUDWidget()
{
	if (!ArenaHUDWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			ArenaHUDWidget = CreateWidget<UDBAArenaHUDRootWidgetBase>(PC, ArenaHUDWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateGameSettingsWidget()
{
	if (!GameSettingsWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			GameSettingsWidget = CreateWidget<UDBAGameSettingsWidgetBase>(PC, GameSettingsWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateInventoryWidget()
{
	if (!InventoryWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			InventoryWidget = CreateWidget<UDBAInventoryWidgetBase>(PC, InventoryWidgetClass);
		}
	}
}

void UDBAGameUIManager::HandleLoginFlowStateChanged(EDBALoginFlowState NewState)
{
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 登录流程状态已变化：%d"), static_cast<int32>(NewState));
	CachedLoginFlowState = NewState;
	RefreshLoginFlowWidgetVisibility();

	if (NewState == EDBALoginFlowState::MainLobby && IsLobbyGameplayWorldForUIManager(GetWorld()))
	{
		TransitionTo(EDBAUIState::Lobby);
	}
}

void UDBAGameUIManager::RefreshLoginFlowWidgetVisibility()
{
	switch (CachedLoginFlowState)
	{
	case EDBALoginFlowState::Startup:
	{
		// Keep startup clean so splash video can appear before login flow widgets.
		HideMainLobby();
		HideAllFlowWidgets();
		ResetFlowWidgetRefreshRetry();
		break;
	}
	case EDBALoginFlowState::LoginScreen:
	case EDBALoginFlowState::TryAutoLogin:
	{
		EnsureLoginFlowBackgroundMusic();
		HideMainLobby();
		if (UUserWidget* Widget = EnsureFlowWidgetCreated(LoginWidgetClass, LoginWidget))
		{
			SetFlowWidgetVisible(Widget);
			ResetFlowWidgetRefreshRetry();
		}
		else
		{
			ScheduleFlowWidgetRefreshRetry();
		}
		break;
	}
	case EDBALoginFlowState::CharacterSelect:
	{
		EnsureLoginFlowBackgroundMusic();
		HideMainLobby();
		if (UUserWidget* SelectWidget = EnsureFlowWidgetCreated(CharacterSelectWidgetClass, CharacterSelectWidget))
		{
			SetFlowWidgetVisible(SelectWidget);
			RemoveAllViewportLoginWidgets(GetWorld());
			ResetFlowWidgetRefreshRetry();
		}
		else
		{
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 角色选择控件尚未就绪，准备重试。"));
			ScheduleFlowWidgetRefreshRetry();
		}
		break;
	}
	case EDBALoginFlowState::CharacterCreate:
	{
		EnsureLoginFlowBackgroundMusic();
		HideMainLobby();
		if (UUserWidget* CreateWidget = EnsureFlowWidgetCreated(CharacterCreateWidgetClass, CharacterCreateWidget))
		{
			SetFlowWidgetVisible(CreateWidget);
			RemoveAllViewportLoginWidgets(GetWorld());
			ResetFlowWidgetRefreshRetry();
		}
		else
		{
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 角色创建控件尚未就绪，准备重试。"));
			ScheduleFlowWidgetRefreshRetry();
		}
		break;
	}
	case EDBALoginFlowState::MainLobby:
	{
		ResetFlowWidgetRefreshRetry();
		StopLoginFlowBackgroundMusic();
		RemoveAllViewportLoginWidgets(GetWorld());
		if (IsLobbyGameplayWorldForUIManager(GetWorld()))
		{
			ShowMainLobby();
		}
		else
		{
			ShowLobbyLoadingScreen();
		}
		break;
	}
	default:
	{
		break;
	}
	}
}

void UDBAGameUIManager::HideAllFlowWidgets()
{
	HideSplashVideo();

	if (LoginWidget)
	{
		LoginWidget->RemoveFromParent();
	}
	if (CharacterSelectWidget)
	{
		CharacterSelectWidget->RemoveFromParent();
	}
	if (CharacterCreateWidget)
	{
		CharacterCreateWidget->RemoveFromParent();
	}
	bFlowWidgetVisible = false;
}

template<typename WidgetType>
WidgetType* UDBAGameUIManager::EnsureFlowWidgetCreated(TSubclassOf<WidgetType> WidgetClass, TObjectPtr<WidgetType>& WidgetInstance)
{
	if (WidgetInstance)
	{
		const UWorld* CurrentWorld = GetWorld();
		const UWorld* WidgetWorld = WidgetInstance->GetWorld();
		const APlayerController* OwningPC = WidgetInstance->GetOwningPlayer();
		const UWorld* OwningPlayerWorld = OwningPC ? OwningPC->GetWorld() : nullptr;
		const bool bWidgetWorldMatches = CurrentWorld && (WidgetWorld == CurrentWorld || OwningPlayerWorld == CurrentWorld);
		if (bWidgetWorldMatches)
		{
			return WidgetInstance;
		}

		WidgetInstance->RemoveFromParent();
		WidgetInstance = nullptr;
	}
	if (!WidgetClass)
	{
		return nullptr;
	}
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC && World->GetGameInstance())
		{
			PC = World->GetGameInstance()->GetPrimaryPlayerController();
		}
		if (PC)
		{
			WidgetInstance = CreateWidget<WidgetType>(PC, WidgetClass);
		}
	}
	return WidgetInstance;
}

void UDBAGameUIManager::ScheduleFlowWidgetRefreshRetry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (World->GetTimerManager().IsTimerActive(FlowWidgetRefreshRetryTimerHandle))
	{
		return;
	}

	++FlowWidgetRefreshRetryCount;
	if (FlowWidgetRefreshRetryCount > 30)
	{
		UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] 流程控件重试超过上限，状态=%d"), static_cast<int32>(CachedLoginFlowState));
		return;
	}

	World->GetTimerManager().SetTimer(
		FlowWidgetRefreshRetryTimerHandle,
		this,
		&UDBAGameUIManager::HandleFlowWidgetRefreshRetry,
		0.15f,
		false);
}

void UDBAGameUIManager::ResetFlowWidgetRefreshRetry()
{
	FlowWidgetRefreshRetryCount = 0;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlowWidgetRefreshRetryTimerHandle);
	}
}

void UDBAGameUIManager::ScheduleLobbyHUDRefreshRetry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (World->GetTimerManager().IsTimerActive(LobbyHUDRefreshRetryTimerHandle))
	{
		return;
	}

	++LobbyHUDRefreshRetryCount;
	if (LobbyHUDRefreshRetryCount > 60)
	{
		UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] 大厅 HUD 重试超过上限。"));
		return;
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 大厅 HUD 尚未就绪，准备第 %d 次重试。"), LobbyHUDRefreshRetryCount);
	World->GetTimerManager().SetTimer(
		LobbyHUDRefreshRetryTimerHandle,
		this,
		&UDBAGameUIManager::ShowLobbyPlayerHUD,
		0.15f,
		false);
}

void UDBAGameUIManager::ResetLobbyHUDRefreshRetry()
{
	LobbyHUDRefreshRetryCount = 0;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LobbyHUDRefreshRetryTimerHandle);
	}
}

void UDBAGameUIManager::CreateLobbyPlayerHUDWidget()
{
	if (!LobbyPlayerHUDWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			LobbyPlayerHUDWidget = CreateWidget<UDBALobbyPlayerHUDWidgetBase>(PC, LobbyPlayerHUDWidgetClass);
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 大厅玩家 HUD 控件已创建：类=%s 成功=%s"),
				*LobbyPlayerHUDWidgetClass->GetName(),
				LobbyPlayerHUDWidget ? TEXT("是") : TEXT("否"));
		}
		else
		{
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 大厅玩家 HUD 正在等待 PlayerController。"));
		}
	}
}

void UDBAGameUIManager::HandleFlowWidgetRefreshRetry()
{
	RefreshLoginFlowWidgetVisibility();
}

void UDBAGameUIManager::SetFlowWidgetVisible(UUserWidget* WidgetToShow)
{
	if (!WidgetToShow)
	{
		return;
	}

	HideAllFlowWidgets();
	WidgetToShow->AddToViewport(10);
	ApplyFrontendInputMode(GetWorld(), WidgetToShow);
	bFlowWidgetVisible = true;
}

void UDBAGameUIManager::ShowSplashVideo()
{
	if (IsServerLikeRuntime(GetWorld()))
	{
		return;
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 请求显示启动视频，控件类=%s"), SplashVideoWidgetClass ? TEXT("有效") : TEXT("空"));

	if (!SplashVideoWidget)
	{
		if (!SplashVideoWidgetClass)
		{
			UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] 启动视频控件类为空。"));
			return;
		}
		if (UWorld* World = GetWorld())
		{
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 世界=%s URL=%s PlayerController数量=%d"), *World->GetName(), *World->URL.ToString(), World->GetNumPlayerControllers());
			APlayerController* PC = World->GetGameInstance() ? World->GetGameInstance()->GetPrimaryPlayerController() : nullptr;
			if (!PC)
			{
				PC = World->GetFirstPlayerController();
			}
			if (PC)
			{
				SplashVideoWidget = CreateWidget<UDBASplashVideoWidget>(PC, SplashVideoWidgetClass);
				UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 启动视频控件创建结果：%s"), SplashVideoWidget ? TEXT("成功") : TEXT("失败"));
			}
			else
			{
				UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 世界 %s 中没有找到 PlayerController，当前数量=%d"), *World->GetName(), World->GetNumPlayerControllers());
				return;
			}
		}
		else
		{
			UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] 没有找到世界对象。"));
		}
	}
	if (SplashVideoWidget)
	{
		RemoveAllViewportLoginWidgets(GetWorld());
		if (CharacterSelectWidget)
		{
			CharacterSelectWidget->RemoveFromParent();
		}
		if (CharacterCreateWidget)
		{
			CharacterCreateWidget->RemoveFromParent();
		}
		bFlowWidgetVisible = false;

		SplashVideoWidget->AddToViewport(999);
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 启动视频控件已添加到视口。"));

		// 设置键盘焦点到启动视频控件，以便接收 ESC 按键
		ApplySplashInputMode(GetWorld(), SplashVideoWidget);
	}
}

void UDBAGameUIManager::HideSplashVideo()
{
	if (SplashVideoWidget && SplashVideoWidget->IsInViewport())
	{
		SplashVideoWidget->RemoveFromParent();
	}
}

void UDBAGameUIManager::EnsureLoginFlowBackgroundMusic()
{
	if (IsServerLikeRuntime(GetWorld()))
	{
		return;
	}

	if (LoginFlowBackgroundMusicComponent)
	{
		return;
	}

	if (!LoginFlowBackgroundMusicSound)
	{
		LoginFlowBackgroundMusicSound = LoadAssetIfCookedAvailable_UIManager<USoundBase>(TEXT("/Game/DBA/Audio/UI/BGM/BGM_LoginFlow_Loop.BGM_LoginFlow_Loop"));
		if (!LoginFlowBackgroundMusicSound)
		{
			LoginFlowBackgroundMusicSound = LoadAssetIfCookedAvailable_UIManager<USoundBase>(TEXT("/Game/DBA/Audio/UI/BGM/BGM_Login_Loop.BGM_Login_Loop"));
		}
	}

	if (!LoginFlowBackgroundMusicSound || !GetWorld())
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 登录流程背景音乐不可用。"));
		return;
	}

	LoginFlowBackgroundMusicComponent = UGameplayStatics::SpawnSound2D(GetWorld(), LoginFlowBackgroundMusicSound, 0.72f, 1.0f, 0.0f, nullptr, false, false);
	if (!LoginFlowBackgroundMusicComponent)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 启动登录流程背景音乐失败。"));
		return;
	}

	LoginFlowBackgroundMusicComponent->bIsUISound = true;
	LoginFlowBackgroundMusicComponent->OnAudioFinished.RemoveDynamic(this, &UDBAGameUIManager::HandleLoginFlowBackgroundMusicFinished);
	LoginFlowBackgroundMusicComponent->OnAudioFinished.AddDynamic(this, &UDBAGameUIManager::HandleLoginFlowBackgroundMusicFinished);
}

void UDBAGameUIManager::StopLoginFlowBackgroundMusic()
{
	if (!LoginFlowBackgroundMusicComponent)
	{
		return;
	}

	LoginFlowBackgroundMusicComponent->OnAudioFinished.RemoveDynamic(this, &UDBAGameUIManager::HandleLoginFlowBackgroundMusicFinished);
	LoginFlowBackgroundMusicComponent->Stop();
	LoginFlowBackgroundMusicComponent = nullptr;
}

void UDBAGameUIManager::HandleLoginFlowBackgroundMusicFinished()
{
	LoginFlowBackgroundMusicComponent = nullptr;
	EnsureLoginFlowBackgroundMusic();
}
