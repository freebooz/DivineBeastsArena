// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Subsystems/DBAUILayerManagerSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "GameDBA/UI/Framework/DBACommonModalBase.h"
#include "GameDBA/UI/Framework/DBAUIRootLayout.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameDBA/Frontend/ServerDirectory/DBAServerSelectScreenBase.h"
#include "GameDBA/Frontend/Settings/DBAFrontendSettings.h"
#include "GameDBA/UI/Widgets/Common/DBAErrorBannerWidgetBase.h"
#include "GameDBA/UI/Widgets/Common/DBAGlobalLoadingWidgetBase.h"
#include "GameDBA/UI/Widgets/Common/DBANetworkStatusWidgetBase.h"
#include "GameDBA/UI/Widgets/Common/DBASystemToastWidgetBase.h"
#include "GameFramework/PlayerController.h"
#include "GameCore/Core/DBALogChannels.h"

void UDBAUILayerManagerSubsystem::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();
	BindFrontendFlow();
}

void UDBAUILayerManagerSubsystem::OnSubsystemDeinitialize()
{
	if (FrontendFlow.IsValid())
	{
		FrontendFlow->OnFrontendStateChanged.RemoveDynamic(this, &UDBAUILayerManagerSubsystem::HandleFrontendStateChanged);
	}
	FrontendFlow.Reset();
	ActiveLoadingRequestTokens.Empty();
	if (RootLayout)
	{
		RootLayout->RemoveFromParent();
	}

	RootLayout = nullptr;
	GlobalLoadingWidget = nullptr;
	SystemToastWidget = nullptr;
	ErrorBannerWidget = nullptr;
	NetworkStatusWidget = nullptr;
	ServerSelectScreen = nullptr;
	Super::OnSubsystemDeinitialize();
}

void UDBAUILayerManagerSubsystem::BindFrontendFlow()
{
	UDBAFrontendFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr;
	if (!Flow || FrontendFlow.Get() == Flow)
	{
		return;
	}

	if (FrontendFlow.IsValid())
	{
		FrontendFlow->OnFrontendStateChanged.RemoveDynamic(this, &UDBAUILayerManagerSubsystem::HandleFrontendStateChanged);
	}
	FrontendFlow = Flow;
	Flow->OnFrontendStateChanged.AddDynamic(this, &UDBAUILayerManagerSubsystem::HandleFrontendStateChanged);
}

void UDBAUILayerManagerSubsystem::HandleFrontendStateChanged(const EDBAFrontendState PreviousState, const EDBAFrontendState NewState)
{
	if (NewState == EDBAFrontendState::ServerSelect)
	{
		ActivateServerSelectScreen();
		return;
	}

	if (ServerSelectScreen && PreviousState == EDBAFrontendState::ServerSelect)
	{
		RemoveWidget(ServerSelectScreen);
	}
}

bool UDBAUILayerManagerSubsystem::ActivateServerSelectScreen()
{
	if (!IsClientUIRuntime())
	{
		return false;
	}

	if (!ServerSelectScreen)
	{
		TSubclassOf<UDBAServerSelectScreenBase> ScreenClass = UDBAServerSelectScreenBase::StaticClass();
		const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
		if (Settings && !Settings->ServerSelectScreenWidgetClass.IsNull())
		{
			if (UClass* ConfiguredClass = Settings->ServerSelectScreenWidgetClass.LoadSynchronous())
			{
				if (ConfiguredClass->IsChildOf(UDBAServerSelectScreenBase::StaticClass()))
				{
					ScreenClass = ConfiguredClass;
				}
				else
				{
					UE_LOG(LogDBAFrontend, Warning, TEXT("选服界面配置类不是 UDBAServerSelectScreenBase 的子类，已使用 C++ 回退页。"));
				}
			}
		}

		ServerSelectScreen = Cast<UDBAServerSelectScreenBase>(CreateWidgetForLocalPlayer(ScreenClass));
	}

	if (!ServerSelectScreen || !MountWidget(ServerSelectScreen, EDBAUILayer::Screen))
	{
		UE_LOG(LogDBAFrontend, Error, TEXT("无法激活选服界面。"));
		return false;
	}

	ServerSelectScreen->ActivateScreen();
	return true;
}

bool UDBAUILayerManagerSubsystem::IsClientUIRuntime() const
{
	const UWorld* World = GetWorld();
	return World && !IsRunningDedicatedServer() && World->GetNetMode() != NM_DedicatedServer;
}

UUserWidget* UDBAUILayerManagerSubsystem::CreateWidgetForLocalPlayer(TSubclassOf<UUserWidget> WidgetClass) const
{
	if (!WidgetClass || !IsClientUIRuntime())
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	return PlayerController ? CreateWidget<UUserWidget>(PlayerController, WidgetClass) : nullptr;
}

bool UDBAUILayerManagerSubsystem::EnsureRootLayout()
{
	if (RootLayout)
	{
		return true;
	}

	if (!IsClientUIRuntime())
	{
		UE_LOG(LogDBAFrontend, Verbose, TEXT("UI 分层管理器跳过 Dedicated Server 或无效世界的根布局创建。"));
		return false;
	}

	UUserWidget* Widget = CreateWidgetForLocalPlayer(UDBAUIRootLayout::StaticClass());
	RootLayout = Cast<UDBAUIRootLayout>(Widget);
	if (!RootLayout)
	{
		UE_LOG(LogDBAFrontend, Error, TEXT("UI 根布局创建失败，业务界面不会直接加入视口。"));
		return false;
	}

	// The only viewport attachment in the frontend UI system.
	RootLayout->AddToViewport(0);
	BindFrontendFlow();
	if (FrontendFlow.IsValid() && FrontendFlow->GetFrontendState() == EDBAFrontendState::ServerSelect)
	{
		ActivateServerSelectScreen();
	}
	UE_LOG(LogDBAFrontend, Log, TEXT("已创建前台 UI 根布局及 Background/Screen/Modal/Toast/Tooltip/Debug 分层。"));
	return true;
}

bool UDBAUILayerManagerSubsystem::MountWidget(UUserWidget* Widget, EDBAUILayer Layer)
{
	if (!Widget || !EnsureRootLayout())
	{
		return false;
	}

	switch (Layer)
	{
	case EDBAUILayer::Background: return RootLayout->MountBackground(Widget);
	case EDBAUILayer::Screen: return RootLayout->MountScreen(Widget);
	case EDBAUILayer::Modal: return RootLayout->MountModal(Widget);
	case EDBAUILayer::Toast: return RootLayout->MountToast(Widget);
	case EDBAUILayer::Tooltip: return RootLayout->MountTooltip(Widget);
	case EDBAUILayer::Debug: return RootLayout->MountDebug(Widget);
	default: return false;
	}
}

bool UDBAUILayerManagerSubsystem::RemoveWidget(UUserWidget* Widget)
{
	return RootLayout && RootLayout->RemoveManagedWidget(Widget);
}

FName UDBAUILayerManagerSubsystem::BeginGlobalLoading(const FText& Message)
{
	if (!EnsureRootLayout())
	{
		return NAME_None;
	}

	const FName RequestToken(*FGuid::NewGuid().ToString(EGuidFormats::Digits));
	ActiveLoadingRequestTokens.Add(RequestToken);

	if (!GlobalLoadingWidget)
	{
		GlobalLoadingWidget = Cast<UDBAGlobalLoadingWidgetBase>(CreateWidgetForLocalPlayer(UDBAGlobalLoadingWidgetBase::StaticClass()));
	}

	if (GlobalLoadingWidget)
	{
		GlobalLoadingWidget->SetLoadingMessage(Message);
		MountWidget(GlobalLoadingWidget, EDBAUILayer::Modal);
		ApplyUIInputMode(GlobalLoadingWidget);
	}

	return RequestToken;
}

bool UDBAUILayerManagerSubsystem::EndGlobalLoading(FName RequestToken)
{
	if (RequestToken.IsNone() || ActiveLoadingRequestTokens.Remove(RequestToken) == 0)
	{
		UE_LOG(LogDBAFrontend, Warning, TEXT("忽略未知的全局加载请求令牌。"));
		return false;
	}

	if (ActiveLoadingRequestTokens.IsEmpty())
	{
		RemoveWidget(GlobalLoadingWidget);
		RestoreGameInputMode();
	}

	return true;
}

void UDBAUILayerManagerSubsystem::ShowToast(const FText& Message)
{
	if (!EnsureRootLayout())
	{
		return;
	}

	if (!SystemToastWidget)
	{
		SystemToastWidget = Cast<UDBASystemToastWidgetBase>(CreateWidgetForLocalPlayer(UDBASystemToastWidgetBase::StaticClass()));
	}

	if (SystemToastWidget)
	{
		SystemToastWidget->ShowMessage(Message);
		MountWidget(SystemToastWidget, EDBAUILayer::Toast);
	}
}

void UDBAUILayerManagerSubsystem::ShowErrorBanner(const FText& Message)
{
	if (!EnsureRootLayout())
	{
		return;
	}

	if (!ErrorBannerWidget)
	{
		ErrorBannerWidget = Cast<UDBAErrorBannerWidgetBase>(CreateWidgetForLocalPlayer(UDBAErrorBannerWidgetBase::StaticClass()));
	}

	if (ErrorBannerWidget)
	{
		ErrorBannerWidget->ShowError(Message);
		MountWidget(ErrorBannerWidget, EDBAUILayer::Toast);
	}
}

void UDBAUILayerManagerSubsystem::SetNetworkAvailable(bool bAvailable, const FText& StatusText)
{
	if (!EnsureRootLayout())
	{
		return;
	}

	if (!NetworkStatusWidget)
	{
		NetworkStatusWidget = Cast<UDBANetworkStatusWidgetBase>(CreateWidgetForLocalPlayer(UDBANetworkStatusWidgetBase::StaticClass()));
	}

	if (NetworkStatusWidget)
	{
		NetworkStatusWidget->SetNetworkStatus(bAvailable, StatusText);
		MountWidget(NetworkStatusWidget, EDBAUILayer::Toast);
	}
}

bool UDBAUILayerManagerSubsystem::HandleBackAction()
{
	if (!RootLayout)
	{
		return false;
	}

	if (UDBACommonModalBase* Modal = Cast<UDBACommonModalBase>(RootLayout->GetTopModal()))
	{
		Modal->Dismiss();
		return true;
	}

	return false;
}

void UDBAUILayerManagerSubsystem::ApplyUIInputMode(UUserWidget* FocusWidget) const
{
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController || !FocusWidget)
	{
		return;
	}

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
}

void UDBAUILayerManagerSubsystem::RestoreGameInputMode() const
{
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(false);
}
