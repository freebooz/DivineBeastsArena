// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/DBAGameUIManager.h"
#include "GameDBA/UI/Splash/UDBASplashVideoWidget.h"
#include "GameDBA/Core/DBALogChannels.h"

#include "Blueprint/UserWidget.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameDBA/UI/Arena/UDBAArenaHUDRootWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.h"
#include "UObject/ConstructorHelpers.h"

UDBAGameUIManager::UDBAGameUIManager()
	: Super()
{
	static ConstructorHelpers::FClassFinder<UDBAMainLobbyWidgetBase> MainLobbyWidgetFinder(TEXT("/Game/UI/Lobby/MainLobby/WBP_DBA_MainLobby"));
	if (MainLobbyWidgetFinder.Succeeded())
	{
		MainLobbyWidgetClass = MainLobbyWidgetFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UDBAArenaHUDRootWidgetBase> ArenaHudWidgetFinder(TEXT("/Game/UI/Arena/HUD/WBP_DBA_ArenaHUDRoot"));
	if (ArenaHudWidgetFinder.Succeeded())
	{
		ArenaHUDWidgetClass = ArenaHudWidgetFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UDBALoginFlowWidgetBase> LoginWidgetFinder(TEXT("/Game/UI/Lobby/Login/WBP_DBA_Login"));
	if (LoginWidgetFinder.Succeeded())
	{
		LoginWidgetClass = LoginWidgetFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UDBACharacterSelectFlowWidgetBase> CharacterSelectWidgetFinder(TEXT("/Game/UI/Lobby/Character/WBP_DBA_CharacterSelect"));
	if (CharacterSelectWidgetFinder.Succeeded())
	{
		CharacterSelectWidgetClass = CharacterSelectWidgetFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UDBACharacterCreateFlowWidgetBase> CharacterCreateWidgetFinder(TEXT("/Game/UI/Lobby/Character/WBP_DBA_CharacterCreate"));
	if (CharacterCreateWidgetFinder.Succeeded())
	{
		CharacterCreateWidgetClass = CharacterCreateWidgetFinder.Class;
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

void UDBAGameUIManager::OnSubsystemDeinitialize()
{
	if (UDBALoginFlowSubsystem* LoginFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>() : nullptr)
	{
		LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBAGameUIManager::HandleLoginFlowStateChanged);
	}

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
	HideAllFlowWidgets();

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
	case EDBALoginFlowState::LoginScreen:
	case EDBALoginFlowState::TryAutoLogin:
	{
		HideMainLobby();
		SetFlowWidgetVisible(EnsureFlowWidgetCreated(LoginWidgetClass, LoginWidget));
		break;
	}
	case EDBALoginFlowState::CharacterSelect:
	{
		HideMainLobby();
		SetFlowWidgetVisible(EnsureFlowWidgetCreated(CharacterSelectWidgetClass, CharacterSelectWidget));
		break;
	}
	case EDBALoginFlowState::CharacterCreate:
	{
		HideMainLobby();
		SetFlowWidgetVisible(EnsureFlowWidgetCreated(CharacterCreateWidgetClass, CharacterCreateWidget));
		break;
	}
	case EDBALoginFlowState::MainLobby:
	{
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
		return WidgetInstance;
	}
	if (!WidgetClass)
	{
		return nullptr;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			WidgetInstance = CreateWidget<WidgetType>(PC, WidgetClass);
		}
	}
	return WidgetInstance;
}

void UDBAGameUIManager::SetFlowWidgetVisible(UUserWidget* WidgetToShow)
{
	if (!WidgetToShow)
	{
		return;
	}

	HideAllFlowWidgets();
	WidgetToShow->AddToViewport(10);
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
	if (SplashVideoWidget && !bFlowWidgetVisible)
	{
		SplashVideoWidget->AddToViewport(999);
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] SplashVideoWidget added to viewport"));

		// 设置键盘焦点到启动视频控件，以便接收 ESC 按键
		SplashVideoWidget->SetFocus();
	}
}

void UDBAGameUIManager::HideSplashVideo()
{
	if (SplashVideoWidget)
	{
		SplashVideoWidget->RemoveFromParent();
	}
}
