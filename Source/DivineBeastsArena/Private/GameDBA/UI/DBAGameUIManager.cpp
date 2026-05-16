// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/DBAGameUIManager.h"
#include "GameDBA/UI/Splash/UDBASplashVideoWidget.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GameInstance/DBAGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameDBA/UI/Arena/UDBAArenaHUDRootWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Misc/PackageName.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	template<typename WidgetType>
	TSubclassOf<WidgetType> ResolveWidgetClassPath(std::initializer_list<const TCHAR*> CandidatePaths)
	{
		for (const TCHAR* CandidatePath : CandidatePaths)
		{
			ConstructorHelpers::FClassFinder<WidgetType> Finder(CandidatePath);
			if (Finder.Succeeded())
			{
				return Finder.Class;
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
		FocusWidget->SetFocus();
	}

	template<typename AssetType>
	AssetType* LoadAssetIfCookedAvailable(const TCHAR* ObjectPath)
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
}

UDBAGameUIManager::UDBAGameUIManager()
	: Super()
{
	MainLobbyWidgetClass = ResolveWidgetClassPath<UDBAMainLobbyWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/MainLobby/WBP_DBA_MainLobby"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_MainLobby")
	});
	if (!MainLobbyWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Main lobby widget blueprint unavailable"));
	}

	ArenaHUDWidgetClass = ResolveWidgetClassPath<UDBAArenaHUDRootWidgetBase>({
		TEXT("/Game/UI/Arena/HUD/WBP_DBA_ArenaHUDRoot"),
		TEXT("/Game/Blueprints/UI/DBA/ArenaHUD/WBP_DBA_ArenaHUDRoot")
	});
	if (!ArenaHUDWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Arena HUD widget blueprint unavailable"));
	}

	LoginWidgetClass = ResolveWidgetClassPath<UDBALoginFlowWidgetBase>({
		TEXT("/Game/DBA/UI/Frontend/Login/WBP_DBA_Login")
	});
	if (!LoginWidgetClass)
	{
		LoginWidgetClass = UDBALoginFlowWidgetBase::StaticClass();
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Login widget blueprint unavailable, using native fallback"));
	}

	CharacterSelectWidgetClass = ResolveWidgetClassPath<UDBACharacterSelectFlowWidgetBase>({
		TEXT("/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterSelect"),
		TEXT("/Game/DBA/UI/Lobby/Character/WBP_DBA_CharacterSelect"),
		TEXT("/Game/Blueprints/UI/DBA/Login/WBP_DBA_CharacterSelect")
	});
	if (!CharacterSelectWidgetClass)
	{
		CharacterSelectWidgetClass = UDBACharacterSelectFlowWidgetBase::StaticClass();
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Character select widget blueprint unavailable, using native fallback"));
	}

	CharacterCreateWidgetClass = ResolveWidgetClassPath<UDBACharacterCreateFlowWidgetBase>({
		TEXT("/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterCreate"),
		TEXT("/Game/DBA/UI/Lobby/Character/WBP_DBA_CharacterCreate"),
		TEXT("/Game/Blueprints/UI/DBA/Login/WBP_DBA_CharacterCreate")
	});
	if (!CharacterCreateWidgetClass)
	{
		CharacterCreateWidgetClass = UDBACharacterCreateFlowWidgetBase::StaticClass();
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Character create widget blueprint unavailable, using native fallback"));
	}

	static ConstructorHelpers::FClassFinder<UDBASplashVideoWidget> SplashVideoWidgetFinder(TEXT("/Game/UI/Splash/WBP_DBA_SplashVideo"));
	if (SplashVideoWidgetFinder.Succeeded())
	{
		SplashVideoWidgetClass = SplashVideoWidgetFinder.Class;
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] SplashVideoWidgetClass found: %s"), *SplashVideoWidgetClass->GetName());
	}
	else
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] SplashVideoWidgetFinder failed to find class"));
	}
}

void UDBAGameUIManager::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();

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
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] Waiting for PlayerController... World: %s"), *World->GetName());
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
		MainLobbyWidget->AddToViewport(0);
		bMainLobbyVisible = true;
	}
}

void UDBAGameUIManager::HideMainLobby()
{
	if (MainLobbyWidget && bMainLobbyVisible)
	{
		MainLobbyWidget->RemoveFromParent();
		bMainLobbyVisible = false;
	}
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
	HideAllFlowWidgets();
	HideMainLobby();
	HideArenaHUD();
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

void UDBAGameUIManager::HandleLoginFlowStateChanged(EDBALoginFlowState NewState)
{
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] Login flow state changed: %d"), static_cast<int32>(NewState));
	CachedLoginFlowState = NewState;
	RefreshLoginFlowWidgetVisibility();

	if (NewState == EDBALoginFlowState::MainLobby)
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
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] CharacterSelect widget not ready, scheduling retry."));
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
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] CharacterCreate widget not ready, scheduling retry."));
			ScheduleFlowWidgetRefreshRetry();
		}
		break;
	}
	case EDBALoginFlowState::MainLobby:
	{
		ResetFlowWidgetRefreshRetry();
		StopLoginFlowBackgroundMusic();
		RemoveAllViewportLoginWidgets(GetWorld());
		HideAllFlowWidgets();
		ShowMainLobby();
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
		UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] Flow widget retry exceeded limit at state=%d"), static_cast<int32>(CachedLoginFlowState));
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
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] ShowSplashVideo called, Class: %s"), SplashVideoWidgetClass ? TEXT("Valid") : TEXT("NULL"));

	if (!SplashVideoWidget)
	{
		if (!SplashVideoWidgetClass)
		{
			UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] SplashVideoWidgetClass is NULL!"));
			return;
		}
		if (UWorld* World = GetWorld())
		{
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] World: %s, URL: %s, PlayerControllers: %d"), *World->GetName(), *World->URL.ToString(), World->GetNumPlayerControllers());
			APlayerController* PC = World->GetGameInstance() ? World->GetGameInstance()->GetPrimaryPlayerController() : nullptr;
			if (!PC)
			{
				PC = World->GetFirstPlayerController();
			}
			if (PC)
			{
				SplashVideoWidget = CreateWidget<UDBASplashVideoWidget>(PC, SplashVideoWidgetClass);
				UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] SplashVideoWidget created: %s"), SplashVideoWidget ? TEXT("Success") : TEXT("Failed"));
			}
			else
			{
				UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] No PlayerController found in world '%s' (%d controllers)"), *World->GetName(), World->GetNumPlayerControllers());
				return;
			}
		}
		else
		{
			UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] No World found"));
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
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] SplashVideoWidget added to viewport"));

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
	if (LoginFlowBackgroundMusicComponent)
	{
		return;
	}

	if (!LoginFlowBackgroundMusicSound)
	{
		LoginFlowBackgroundMusicSound = LoadAssetIfCookedAvailable<USoundBase>(TEXT("/Game/DBA/Audio/UI/BGM/BGM_LoginFlow_Loop.BGM_LoginFlow_Loop"));
		if (!LoginFlowBackgroundMusicSound)
		{
			LoginFlowBackgroundMusicSound = LoadAssetIfCookedAvailable<USoundBase>(TEXT("/Game/DBA/Audio/UI/BGM/BGM_Login_Loop.BGM_Login_Loop"));
		}
	}

	if (!LoginFlowBackgroundMusicSound || !GetWorld())
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Login flow BGM unavailable."));
		return;
	}

	LoginFlowBackgroundMusicComponent = UGameplayStatics::SpawnSound2D(GetWorld(), LoginFlowBackgroundMusicSound, 0.72f, 1.0f, 0.0f, nullptr, false, false);
	if (!LoginFlowBackgroundMusicComponent)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Failed to start login flow BGM."));
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
