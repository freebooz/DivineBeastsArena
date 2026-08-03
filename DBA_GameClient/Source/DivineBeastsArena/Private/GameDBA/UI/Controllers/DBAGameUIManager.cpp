// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Controllers/DBAGameUIManager.h"
#include "GameDBA/UI/Widgets/Splash/UDBASplashVideoWidget.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Framework/GameInstance/DBAGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendTelemetryService.h"
#include "GameDBA/Frontend/Account/DBAOnlineAccountService.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameDBA/UI/Widgets/Arena/UDBAArenaHUDRootWidgetBase.h"
#include "GameDBA/UI/Controllers/Arena/UDBAArenaHUDWidgetController.h"
#include "GameDBA/Characters/DBAZodiacCharacterBase.h"
#include "GameDBA/Frontend/Lobby/UDBAMainLobbyWidgetBase.h"
#include "GameDBA/Frontend/Lobby/UDBALobbyPlayerHUDWidgetBase.h"
#include "GameDBA/Frontend/Auth/UDBALoginFlowWidgetBase.h"
#include "GameDBA/Frontend/CharacterSelection/UDBACharacterSelectFlowWidgetBase.h"
#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateFlowWidgetBase.h"
#include "GameDBA/Frontend/DBAFrontendEnvironmentSubsystem.h"
#include "GameDBA/Data/Registries/DBAUIFlowRegistry.h"
#include "GameDBA/UI/DBAUIFontUtils.h"
#include "GameDBA/UI/DBAUIDeveloperSettings.h"
#include "GameDBA/UI/Frontend/DBAFrontendFlowController.h"
#include "GameDBA/UI/Widgets/Loading/UDBALoadingScreenWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAGameSettingsWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAInventoryWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAInvitePanelWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAInteractionPromptWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAMatchFoundWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBANewbieTaskTrackerWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBANewbieVillageMainWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAPartyPanelWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAPortalConfirmWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAQueueModeSelectWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAQueueStatusWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/UDBAReadyCheckWidgetBase.h"
#include "GameDBA/UI/Widgets/Common/UDBASoftwareCursorWidget.h"
#include "Components/AudioComponent.h"
#include "Components/EditableTextBox.h"
#include "Engine/AssetManager.h"
#include "Engine/GameViewportClient.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	FSoftObjectPath NormalizeWidgetClassPath(const FSoftObjectPath& ConfiguredPath)
	{
		if (ConfiguredPath.IsNull())
		{
			return ConfiguredPath;
		}

		const FString PathString = ConfiguredPath.ToString();
		if (PathString.Contains(TEXT(".")))
		{
			return ConfiguredPath;
		}

		const FString AssetName = FPackageName::GetShortName(PathString);
		return FSoftObjectPath(FString::Printf(TEXT("%s.%s_C"), *PathString, *AssetName));
	}

	bool HasCompletedAccountLogin(const UGameInstance* GameInstance)
	{
		const UDBAOnlineAccountService* AccountService = GameInstance ? GameInstance->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
		return AccountService && AccountService->IsLoggedIn();
	}

	bool IsWorldSafeForWidgetCreation(const UWorld* World)
	{
		return World && !World->bIsTearingDown;
	}

	template<typename WidgetType>
	TSubclassOf<WidgetType> ResolveLoadedWidgetClass(
		const TSoftClassPtr<WidgetType>& ConfiguredClass,
		UClass* NativeFallback,
		const TCHAR* WidgetLabel)
	{
		if (UClass* LoadedClass = ConfiguredClass.Get())
		{
			return LoadedClass;
		}

		const FSoftObjectPath NormalizedPath = NormalizeWidgetClassPath(ConfiguredClass.ToSoftObjectPath());
		if (UClass* LoadedClass = Cast<UClass>(NormalizedPath.ResolveObject()))
		{
			return LoadedClass;
		}

		if (!ConfiguredClass.IsNull())
		{
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] %s控件类异步加载失败：%s"), WidgetLabel, *ConfiguredClass.ToString());
		}
		else
		{
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] UI 流程注册表未配置%s控件类。"), WidgetLabel);
		}

		return NativeFallback;
	}

	void ApplyFrontendInputMode(UWorld* World, UUserWidget* FocusWidget)
	{
		if (!IsWorldSafeForWidgetCreation(World) || !IsValid(FocusWidget))
		{
			return;
		}

		APlayerController* PC = World->GetFirstPlayerController();
		if (!IsValid(PC))
		{
			return;
		}

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
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

	void ApplyLoginFrontendInputMode(UWorld* World, UDBALoginFlowWidgetBase* LoginWidget)
	{
		if (!IsWorldSafeForWidgetCreation(World) || !IsValid(LoginWidget))
		{
			return;
		}

		APlayerController* PC = World->GetFirstPlayerController();
		if (!IsValid(PC))
		{
			return;
		}

		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		if (UEditableTextBox* EmailInput = LoginWidget->GetDefaultInputFocusWidget())
		{
			InputMode.SetWidgetToFocus(EmailInput->TakeWidget());
		}
		else
		{
			InputMode.SetWidgetToFocus(LoginWidget->TakeWidget());
		}
		PC->SetInputMode(InputMode);
		PC->CurrentMouseCursor = EMouseCursor::Default;
		PC->DefaultMouseCursor = EMouseCursor::Default;
		PC->SetMouseCursorWidget(EMouseCursor::Default, nullptr);
		PC->SetMouseCursorWidget(EMouseCursor::Crosshairs, nullptr);
		PC->SetMouseCursorWidget(EMouseCursor::Hand, nullptr);
		PC->SetShowMouseCursor(true);
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		LoginWidget->FocusDefaultInput();
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 登录界面已启用系统鼠标指针与 UI 专用输入模式。"));
	}

	void ApplySplashInputMode(UWorld* World, UUserWidget* FocusWidget)
	{
		if (!IsWorldSafeForWidgetCreation(World) || !IsValid(FocusWidget))
		{
			return;
		}

		APlayerController* PC = World->GetFirstPlayerController();
		if (!IsValid(PC))
		{
			return;
		}

		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
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
		if (!IsWorldSafeForWidgetCreation(World))
		{
			return;
		}

		APlayerController* PC = World->GetFirstPlayerController();
		if (!IsValid(PC))
		{
			return;
		}

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
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

	bool IsMobileOrCompactViewport(UUserWidget* Widget, const FVector2D& ViewportSize)
	{
		const FString PlatformName = UGameplayStatics::GetPlatformName();
		const bool bMobilePlatform = PlatformName.Equals(TEXT("Android"), ESearchCase::IgnoreCase)
			|| PlatformName.Equals(TEXT("IOS"), ESearchCase::IgnoreCase)
			|| PlatformName.Equals(TEXT("iOS"), ESearchCase::IgnoreCase);
		const bool bCompactViewport = FMath::Min(ViewportSize.X, ViewportSize.Y) > 0.0f
			&& FMath::Min(ViewportSize.X, ViewportSize.Y) < 720.0f;
		return bMobilePlatform || bCompactViewport || !Widget;
	}

	FVector2D GetWidgetViewportSize(UUserWidget* Widget)
	{
		if (!Widget)
		{
			return FVector2D::ZeroVector;
		}

		FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(Widget);
		if ((ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f) && Widget->GetWorld() && Widget->GetWorld()->GetGameViewport())
		{
			Widget->GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);
		}
		return ViewportSize;
	}

	FVector2D ClampWidgetSizeToViewport(UUserWidget* Widget, const FVector2D& DesiredSize, const FVector2D& MinimumSize = FVector2D(300.0f, 160.0f))
	{
		const FVector2D ViewportSize = GetWidgetViewportSize(Widget);
		if (ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f)
		{
			return DesiredSize;
		}

		const bool bCompact = IsMobileOrCompactViewport(Widget, ViewportSize);
		const FVector2D Margin = bCompact ? FVector2D(28.0f, 28.0f) : FVector2D(96.0f, 72.0f);
		const FVector2D MaxSize(
			FMath::Max(MinimumSize.X, ViewportSize.X - Margin.X),
			FMath::Max(MinimumSize.Y, ViewportSize.Y - Margin.Y));

		return FVector2D(
			FMath::Clamp(DesiredSize.X, MinimumSize.X, MaxSize.X),
			FMath::Clamp(DesiredSize.Y, MinimumSize.Y, MaxSize.Y));
	}

	void PlaceWidgetInViewport(UUserWidget* Widget, const FAnchors& Anchors, const FVector2D& Alignment, const FVector2D& Position, const FVector2D& DesiredSize, const FVector2D& MinimumSize = FVector2D(300.0f, 160.0f))
	{
		if (!Widget)
		{
			return;
		}

		const float UIScale = DBAUIFonts::GetViewportUIScale(Widget);
		Widget->SetAnchorsInViewport(Anchors);
		Widget->SetAlignmentInViewport(Alignment);
		Widget->SetPositionInViewport(Position * UIScale, false);
		Widget->SetDesiredSizeInViewport(ClampWidgetSizeToViewport(Widget, DesiredSize * UIScale, MinimumSize * UIScale));
	}

	void CenterModalWidgetInViewport(UUserWidget* Widget, const FVector2D& DesiredSize)
	{
		PlaceWidgetInViewport(
			Widget,
			FAnchors(0.5f, 0.5f),
			FVector2D(0.5f, 0.5f),
			FVector2D::ZeroVector,
			DesiredSize,
			FVector2D(320.0f, 180.0f));
	}
}

UDBAGameUIManager::UDBAGameUIManager()
	: Super()
{
}

void UDBAGameUIManager::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();
	bIsDeinitializing = false;
	bFrontendUIInitializationCompleted = false;

	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 服务器运行环境跳过前端 UI 初始化。"));
		return;
	}

	if (UDBAFrontendFlowSubsystem* LoginFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr)
	{
		FrontendFlowController = NewObject<UDBAFrontendFlowController>(this);
		FrontendFlowController->Initialize(LoginFlow);
		FrontendFlowController->OnViewStateChanged.AddUObject(this, &UDBAGameUIManager::HandleLoginFlowStateChanged);
		CachedLoginFlowState = FrontendFlowController->GetCurrentState();
	}

	RequestUIFlowRegistryAsync();
}

void UDBAGameUIManager::RequestUIFlowRegistryAsync()
{
	const UDBAUIDeveloperSettings* Settings = GetDefault<UDBAUIDeveloperSettings>();
	if (!Settings || Settings->DefaultUIFlowRegistry.IsNull())
	{
		UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] 未配置 UI 流程注册表，前端界面将使用 C++ 原生兜底控件。"));
		ApplyNativeWidgetClassFallbacks();
		CompleteFrontendUIInitialization();
		return;
	}

	if (UDBAUIFlowRegistry* LoadedRegistry = Settings->DefaultUIFlowRegistry.Get())
	{
		UIFlowRegistry = LoadedRegistry;
		HandleUIFlowRegistryLoaded();
		return;
	}

	const FSoftObjectPath RegistryPath = Settings->DefaultUIFlowRegistry.ToSoftObjectPath();
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 开始异步加载 UI 流程注册表：%s"), *RegistryPath.ToString());
	UIFlowRegistryStreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		RegistryPath,
		FStreamableDelegate::CreateUObject(this, &UDBAGameUIManager::HandleUIFlowRegistryLoaded),
		FStreamableManager::AsyncLoadHighPriority,
		true);
}

void UDBAGameUIManager::HandleUIFlowRegistryLoaded()
{
	UIFlowRegistryStreamableHandle.Reset();
	if (bIsDeinitializing)
	{
		return;
	}

	if (!UIFlowRegistry)
	{
		const UDBAUIDeveloperSettings* Settings = GetDefault<UDBAUIDeveloperSettings>();
		UIFlowRegistry = Settings ? Settings->DefaultUIFlowRegistry.Get() : nullptr;
	}

	if (!UIFlowRegistry)
	{
		UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] UI 流程注册表异步加载失败，前端界面将使用 C++ 原生兜底控件。"));
		ApplyNativeWidgetClassFallbacks();
		CompleteFrontendUIInitialization();
		return;
	}

	TArray<FSoftObjectPath> WidgetClassPaths;
	UIFlowRegistry->GetWidgetClassPaths(WidgetClassPaths);
	if (WidgetClassPaths.IsEmpty())
	{
		UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] UI 流程注册表没有配置任何 Widget 类。"));
		ApplyUIFlowRegistry();
		CompleteFrontendUIInitialization();
		return;
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] UI 流程注册表加载完成，开始异步加载 %d 个 Widget 类。"), WidgetClassPaths.Num());
	UIWidgetClassesStreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		WidgetClassPaths,
		FStreamableDelegate::CreateUObject(this, &UDBAGameUIManager::HandleUIWidgetClassesLoaded),
		FStreamableManager::AsyncLoadHighPriority,
		true);
}

void UDBAGameUIManager::HandleUIWidgetClassesLoaded()
{
	UIWidgetClassesStreamableHandle.Reset();
	if (bIsDeinitializing)
	{
		return;
	}

	ApplyUIFlowRegistry();
	CompleteFrontendUIInitialization();
}

void UDBAGameUIManager::ApplyUIFlowRegistry()
{
	if (!UIFlowRegistry)
	{
		ApplyNativeWidgetClassFallbacks();
		return;
	}

	LoginWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->LoginWidgetClass, UDBALoginFlowWidgetBase::StaticClass(), TEXT("登录"));
	CharacterSelectWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->CharacterSelectWidgetClass, UDBACharacterSelectFlowWidgetBase::StaticClass(), TEXT("角色选择"));
	CharacterCreateWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->CharacterCreateWidgetClass, UDBACharacterCreateFlowWidgetBase::StaticClass(), TEXT("角色创建"));
	SplashVideoWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->SplashVideoWidgetClass, nullptr, TEXT("启动视频"));
	MainLobbyWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->MainLobbyWidgetClass, nullptr, TEXT("主大厅"));
	LobbyPlayerHUDWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->LobbyPlayerHUDWidgetClass, UDBALobbyPlayerHUDWidgetBase::StaticClass(), TEXT("大厅玩家界面"));
	LobbyLoadingWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->LobbyLoadingWidgetClass, nullptr, TEXT("大厅加载"));
	GameSettingsWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->GameSettingsWidgetClass, UDBAGameSettingsWidgetBase::StaticClass(), TEXT("游戏设置"));
	InventoryWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->InventoryWidgetClass, UDBAInventoryWidgetBase::StaticClass(), TEXT("背包"));
	PartyPanelWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->PartyPanelWidgetClass, nullptr, TEXT("队伍面板"));
	InvitePanelWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->InvitePanelWidgetClass, nullptr, TEXT("邀请面板"));
	QueueModeSelectWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->QueueModeSelectWidgetClass, nullptr, TEXT("队列模式选择"));
	QueueStatusWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->QueueStatusWidgetClass, nullptr, TEXT("队列状态"));
	ReadyCheckWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->ReadyCheckWidgetClass, nullptr, TEXT("准备确认"));
	MatchFoundWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->MatchFoundWidgetClass, nullptr, TEXT("匹配成功提示"));
	PortalConfirmWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->PortalConfirmWidgetClass, nullptr, TEXT("传送门确认"));
	InteractionPromptWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->InteractionPromptWidgetClass, nullptr, TEXT("交互提示"));
	NewbieVillageMainWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->NewbieVillageMainWidgetClass, nullptr, TEXT("新手村主界面"));
	NewbieTaskTrackerWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->NewbieTaskTrackerWidgetClass, nullptr, TEXT("新手任务追踪"));
	ArenaHUDWidgetClass = ResolveLoadedWidgetClass(UIFlowRegistry->ArenaHUDWidgetClass, nullptr, TEXT("竞技场界面"));
}

void UDBAGameUIManager::ApplyNativeWidgetClassFallbacks()
{
	LoginWidgetClass = UDBALoginFlowWidgetBase::StaticClass();
	CharacterSelectWidgetClass = UDBACharacterSelectFlowWidgetBase::StaticClass();
	CharacterCreateWidgetClass = UDBACharacterCreateFlowWidgetBase::StaticClass();
	LobbyPlayerHUDWidgetClass = UDBALobbyPlayerHUDWidgetBase::StaticClass();
	GameSettingsWidgetClass = UDBAGameSettingsWidgetBase::StaticClass();
	InventoryWidgetClass = UDBAInventoryWidgetBase::StaticClass();
}

void UDBAGameUIManager::CompleteFrontendUIInitialization()
{
	if (bFrontendUIInitializationCompleted || bIsDeinitializing)
	{
		return;
	}

	bFrontendUIInitializationCompleted = true;
	if (FrontendFlowController)
	{
		CachedLoginFlowState = FrontendFlowController->GetCurrentState();
	}
	RefreshLoginFlowWidgetVisibility();

	if (FParse::Param(FCommandLine::Get(), TEXT("DBASkipSplash")))
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 命令行要求跳过启动视频。"));
		EnsureLoginFlowStartedFromManager();
		return;
	}

	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World))
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] UI 资源加载完成时世界不可用，无法显示启动视频。"));
		return;
	}

	// 资源已加载后再延迟显示，等待本地 PlayerController 就绪。
	World->GetTimerManager().SetTimer(SplashVideoTimerHandle, this, &UDBAGameUIManager::TryShowSplashVideo, 0.5f, true);
}

void UDBAGameUIManager::TryShowSplashVideo()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}
	if (!SplashVideoWidgetClass)
	{
		World->GetTimerManager().ClearTimer(SplashVideoTimerHandle);
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
		World->GetTimerManager().ClearTimer(SplashVideoTimerHandle);
		ShowSplashVideo();
	}
	else
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 正在等待玩家控制器，世界=%s"), *World->GetName());
	}
}

void UDBAGameUIManager::EnsureLoginFlowStartedFromManager()
{
	if (bLoginFlowStartRequested)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (UDBAGameInstance* DBAInstance = GetGameInstance() ? Cast<UDBAGameInstance>(GetGameInstance()) : nullptr)
	{
		DBAInstance->StartLoginFlow();
		bLoginFlowStartRequested = true;
		return;
	}

	if (UDBAFrontendFlowSubsystem* LoginFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr)
	{
		if (LoginFlow->GetFlowState() == EDBALoginFlowState::Booting)
		{
			LoginFlow->StartLoginFlow();
		}
		bLoginFlowStartRequested = true;
	}
}

void UDBAGameUIManager::OnSubsystemDeinitialize()
{
	bIsDeinitializing = true;
	bFrontendUIInitializationCompleted = false;

	if (UIFlowRegistryStreamableHandle.IsValid())
	{
		UIFlowRegistryStreamableHandle->CancelHandle();
		UIFlowRegistryStreamableHandle.Reset();
	}
	if (UIWidgetClassesStreamableHandle.IsValid())
	{
		UIWidgetClassesStreamableHandle->CancelHandle();
		UIWidgetClassesStreamableHandle.Reset();
	}
	UIFlowRegistry = nullptr;

	if (FrontendFlowController)
	{
		FrontendFlowController->OnViewStateChanged.RemoveAll(this);
		FrontendFlowController->Deinitialize();
		FrontendFlowController = nullptr;
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
		if (UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr)
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
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (UDBAGameInstance* DBAInstance = Cast<UDBAGameInstance>(GetGameInstance()))
	{
		if (!UDBAGameInstance::CanEnterLobbyGameplay(DBAInstance))
		{
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 未完成登录/选角，拒绝显示大厅界面。"));
			HideLobbyLoadingScreen();
			DBAInstance->StartLoginFlow();
			return;
		}
	}

	StopLoginFlowBackgroundMusic();
	HideAllFlowWidgets();
	// 无论大厅控件是否创建成功，都先关闭加载遮罩，避免卡在“进入大厅”。
	HideLobbyLoadingScreen();

	if (IsLobbyGameplayWorldForUIManager(World))
	{
		if (MainLobbyWidget && bMainLobbyVisible)
		{
			MainLobbyWidget->RemoveFromParent();
			bMainLobbyVisible = false;
		}

		ShowLobbyPlayerHUD();
		ApplyLobbyGameplayInputMode(World);
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 已为大厅地图显示游戏界面。"));
		return;
	}

	if (MainLobbyWidget)
	{
		const UWorld* WidgetWorld = MainLobbyWidget->GetWorld();
		const APlayerController* OwningPC = MainLobbyWidget->GetOwningPlayer();
		const UWorld* OwningPlayerWorld = OwningPC ? OwningPC->GetWorld() : nullptr;
		const bool bWidgetWorldMatches = WidgetWorld == World || OwningPlayerWorld == World;
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
		DBAUIFonts::ApplyViewportScaledPresentation(MainLobbyWidget);
		bMainLobbyVisible = true;
	}
	else if (!MainLobbyWidget)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 主大厅控件创建失败，已关闭加载遮罩。"));
	}

	ShowLobbyPlayerHUD();
	ApplyLobbyGameplayInputMode(World);
}

void UDBAGameUIManager::HideMainLobby()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (MainLobbyWidget && bMainLobbyVisible)
	{
		MainLobbyWidget->RemoveFromParent();
		bMainLobbyVisible = false;
	}
	HideLobbyPlayerHUD();
}

void UDBAGameUIManager::RequestShowLoginFlowWidget()
{
	if (FrontendFlowController)
	{
		CachedLoginFlowState = FrontendFlowController->GetCurrentState();
	}
	ShowLoginFlowWidget();
}

void UDBAGameUIManager::ShowLoginFlowWidget()
{
	RefreshLoginFlowWidgetVisibility();
}

void UDBAGameUIManager::HideLoginFlowWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	HideAllFlowWidgets();
}

void UDBAGameUIManager::ShowArenaHUD()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

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
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

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
	HidePartyPanel();
	HideInvitePanel();
	HideQueueModeSelect();
	HideQueueStatus();
	HideMatchFound();
	HideReadyCheck();
	HidePortalConfirm();
	HideInteractionPrompt();
	HideNewbieVillageMain();
	HideNewbieTaskTracker();
}

void UDBAGameUIManager::ShowLobbyLoadingScreen()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
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
		LobbyLoadingWidget->ShowTips(NSLOCTEXT("DBAGameUIManager", "EnteringLobby", "正在进入大厅..."));
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
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
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

	RestoreInputModeAfterOverlayClosed();
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
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
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

	RestoreInputModeAfterOverlayClosed();
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

void UDBAGameUIManager::ShowPartyPanel()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (UDBAPartyPanelWidgetBase* Widget = EnsureFlowWidgetCreated(PartyPanelWidgetClass, PartyPanelWidget))
	{
		if (!bPartyPanelVisible)
		{
			Widget->AddToViewport(6200);
			CenterModalWidgetInViewport(Widget, FVector2D(720.0f, 520.0f));
			bPartyPanelVisible = true;
		}
		Widget->RefreshPartyMembers();
		ApplyFrontendInputMode(World, Widget);
	}
}

void UDBAGameUIManager::HidePartyPanel()
{
	if (PartyPanelWidget && bPartyPanelVisible)
	{
		PartyPanelWidget->RemoveFromParent();
	}
	bPartyPanelVisible = false;
	RestoreInputModeAfterOverlayClosed();
}

void UDBAGameUIManager::TogglePartyPanel()
{
	bPartyPanelVisible ? HidePartyPanel() : ShowPartyPanel();
}

void UDBAGameUIManager::ShowInvitePanel()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (UDBAInvitePanelWidgetBase* Widget = EnsureFlowWidgetCreated(InvitePanelWidgetClass, InvitePanelWidget))
	{
		if (!bInvitePanelVisible)
		{
			Widget->AddToViewport(6300);
			CenterModalWidgetInViewport(Widget, FVector2D(640.0f, 520.0f));
			bInvitePanelVisible = true;
		}
		Widget->RefreshFriendList();
		ApplyFrontendInputMode(World, Widget);
	}
}

void UDBAGameUIManager::HideInvitePanel()
{
	if (InvitePanelWidget && bInvitePanelVisible)
	{
		InvitePanelWidget->RemoveFromParent();
	}
	bInvitePanelVisible = false;
	RestoreInputModeAfterOverlayClosed();
}

void UDBAGameUIManager::ToggleInvitePanel()
{
	bInvitePanelVisible ? HideInvitePanel() : ShowInvitePanel();
}

void UDBAGameUIManager::ShowQueueModeSelect()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (UDBAQueueModeSelectWidgetBase* Widget = EnsureFlowWidgetCreated(QueueModeSelectWidgetClass, QueueModeSelectWidget))
	{
		if (!bQueueModeSelectVisible)
		{
			Widget->AddToViewport(6400);
			CenterModalWidgetInViewport(Widget, FVector2D(760.0f, 540.0f));
			bQueueModeSelectVisible = true;
		}
		Widget->RefreshModeList();
		ApplyFrontendInputMode(World, Widget);
	}
}

void UDBAGameUIManager::HideQueueModeSelect()
{
	if (QueueModeSelectWidget && bQueueModeSelectVisible)
	{
		QueueModeSelectWidget->RemoveFromParent();
	}
	bQueueModeSelectVisible = false;
	RestoreInputModeAfterOverlayClosed();
}

void UDBAGameUIManager::ToggleQueueModeSelect()
{
	bQueueModeSelectVisible ? HideQueueModeSelect() : ShowQueueModeSelect();
}

void UDBAGameUIManager::ShowQueueStatus(const FText& ModeName, const FText& MapName, const FText& EstimatedWaitTime)
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (UDBAQueueStatusWidgetBase* Widget = EnsureFlowWidgetCreated(QueueStatusWidgetClass, QueueStatusWidget))
	{
		if (!bQueueStatusVisible)
		{
			Widget->AddToViewport(5000);
			CenterModalWidgetInViewport(Widget, FVector2D(520.0f, 180.0f));
			bQueueStatusVisible = true;
		}
		Widget->StartQueue(ModeName, MapName, EstimatedWaitTime);
	}
}

void UDBAGameUIManager::HideQueueStatus()
{
	if (QueueStatusWidget && bQueueStatusVisible)
	{
		QueueStatusWidget->RemoveFromParent();
	}
	bQueueStatusVisible = false;
}

void UDBAGameUIManager::ShowMatchFound(const FText& ModeName, const FText& MapName)
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	HideQueueStatus();
	if (UDBAMatchFoundWidgetBase* Widget = EnsureFlowWidgetCreated(MatchFoundWidgetClass, MatchFoundWidget))
	{
		if (!bMatchFoundVisible)
		{
			Widget->AddToViewport(9000);
			CenterModalWidgetInViewport(Widget, FVector2D(560.0f, 260.0f));
			bMatchFoundVisible = true;
		}
		Widget->ShowMatchFound(ModeName, MapName);
		ApplyFrontendInputMode(World, Widget);
	}
}

void UDBAGameUIManager::HideMatchFound()
{
	if (MatchFoundWidget && bMatchFoundVisible)
	{
		MatchFoundWidget->RemoveFromParent();
	}
	bMatchFoundVisible = false;
	RestoreInputModeAfterOverlayClosed();
}

void UDBAGameUIManager::ShowReadyCheck(const FText& ModeName, const FText& MapName, float TimeoutSeconds)
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	HideMatchFound();
	if (UDBAReadyCheckWidgetBase* Widget = EnsureFlowWidgetCreated(ReadyCheckWidgetClass, ReadyCheckWidget))
	{
		if (!bReadyCheckVisible)
		{
			Widget->AddToViewport(9100);
			CenterModalWidgetInViewport(Widget, FVector2D(560.0f, 300.0f));
			bReadyCheckVisible = true;
		}
		Widget->OnReadyCheckCompletedEvent.RemoveDynamic(this, &UDBAGameUIManager::HandleReadyCheckCompleted);
		Widget->OnReadyCheckCompletedEvent.AddDynamic(this, &UDBAGameUIManager::HandleReadyCheckCompleted);
		Widget->ShowReadyCheck(ModeName, MapName, TimeoutSeconds);
		ApplyFrontendInputMode(World, Widget);
	}
}

void UDBAGameUIManager::HideReadyCheck()
{
	if (ReadyCheckWidget)
	{
		ReadyCheckWidget->OnReadyCheckCompletedEvent.RemoveDynamic(this, &UDBAGameUIManager::HandleReadyCheckCompleted);
		if (bReadyCheckVisible)
		{
			ReadyCheckWidget->RemoveFromParent();
		}
	}
	bReadyCheckVisible = false;
	RestoreInputModeAfterOverlayClosed();
}

void UDBAGameUIManager::ShowPortalConfirm(FName DestinationId, const FText& DestinationName, const FText& DestinationDescription, bool bCanTeleport, const FText& ConditionText)
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (UDBAPortalConfirmWidgetBase* Widget = EnsureFlowWidgetCreated(PortalConfirmWidgetClass, PortalConfirmWidget))
	{
		if (!bPortalConfirmVisible)
		{
			Widget->AddToViewport(9200);
			CenterModalWidgetInViewport(Widget, FVector2D(620.0f, 360.0f));
			bPortalConfirmVisible = true;
		}
		Widget->OnPortalConfirmedEvent.RemoveDynamic(this, &UDBAGameUIManager::HandlePortalConfirmed);
		Widget->OnPortalConfirmedEvent.AddDynamic(this, &UDBAGameUIManager::HandlePortalConfirmed);
		Widget->OnPortalCancelledEvent.RemoveDynamic(this, &UDBAGameUIManager::HandlePortalCancelled);
		Widget->OnPortalCancelledEvent.AddDynamic(this, &UDBAGameUIManager::HandlePortalCancelled);
		Widget->ShowConfirm(DestinationId, DestinationName, DestinationDescription, bCanTeleport, ConditionText);
		ApplyFrontendInputMode(World, Widget);
	}
}

void UDBAGameUIManager::HidePortalConfirm()
{
	if (PortalConfirmWidget)
	{
		PortalConfirmWidget->OnPortalConfirmedEvent.RemoveDynamic(this, &UDBAGameUIManager::HandlePortalConfirmed);
		PortalConfirmWidget->OnPortalCancelledEvent.RemoveDynamic(this, &UDBAGameUIManager::HandlePortalCancelled);
		if (bPortalConfirmVisible)
		{
			PortalConfirmWidget->RemoveFromParent();
		}
	}
	bPortalConfirmVisible = false;
	RestoreInputModeAfterOverlayClosed();
}

void UDBAGameUIManager::ShowInteractionPrompt(EDBAInteractionType Type, const FText& ObjectName, const FText& PromptText, bool bCanInteract)
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (UDBAInteractionPromptWidgetBase* Widget = EnsureFlowWidgetCreated(InteractionPromptWidgetClass, InteractionPromptWidget))
	{
		if (!bInteractionPromptVisible)
		{
			Widget->AddToViewport(4200);
			PlaceWidgetInViewport(
				Widget,
				FAnchors(0.5f, 0.82f),
				FVector2D(0.5f, 0.5f),
				FVector2D::ZeroVector,
				FVector2D(520.0f, 108.0f),
				FVector2D(280.0f, 88.0f));
			bInteractionPromptVisible = true;
		}
		Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Widget->ShowPrompt(Type, ObjectName, PromptText, bCanInteract);
	}
}

void UDBAGameUIManager::HideInteractionPrompt()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (InteractionPromptWidget && bInteractionPromptVisible)
	{
		InteractionPromptWidget->HidePrompt();
		InteractionPromptWidget->RemoveFromParent();
	}
	bInteractionPromptVisible = false;
}

void UDBAGameUIManager::UpdateInteractionProgress(float Progress)
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (InteractionPromptWidget && bInteractionPromptVisible)
	{
		InteractionPromptWidget->UpdateInteractionProgress(Progress);
	}
}

void UDBAGameUIManager::ShowNewbieVillageMain()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (UDBANewbieVillageMainWidgetBase* Widget = EnsureFlowWidgetCreated(NewbieVillageMainWidgetClass, NewbieVillageMainWidget))
	{
		if (!bNewbieVillageMainVisible)
		{
			Widget->AddToViewport(6100);
			bNewbieVillageMainVisible = true;
		}
		Widget->RefreshTaskTracker();
		ApplyFrontendInputMode(World, Widget);
	}
}

void UDBAGameUIManager::HideNewbieVillageMain()
{
	if (NewbieVillageMainWidget && bNewbieVillageMainVisible)
	{
		NewbieVillageMainWidget->RemoveFromParent();
	}
	bNewbieVillageMainVisible = false;
	RestoreInputModeAfterOverlayClosed();
}

void UDBAGameUIManager::ShowNewbieTaskTracker()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (UDBANewbieTaskTrackerWidgetBase* Widget = EnsureFlowWidgetCreated(NewbieTaskTrackerWidgetClass, NewbieTaskTrackerWidget))
	{
		if (!bNewbieTaskTrackerVisible)
		{
			Widget->AddToViewport(4100);
			PlaceWidgetInViewport(
				Widget,
				FAnchors(1.0f, 0.0f),
				FVector2D(1.0f, 0.0f),
				FVector2D(-18.0f, 96.0f),
				FVector2D(340.0f, 260.0f),
				FVector2D(260.0f, 180.0f));
			bNewbieTaskTrackerVisible = true;
		}
		Widget->RefreshTaskList();
	}
}

void UDBAGameUIManager::HideNewbieTaskTracker()
{
	if (NewbieTaskTrackerWidget && bNewbieTaskTrackerVisible)
	{
		NewbieTaskTrackerWidget->RemoveFromParent();
	}
	bNewbieTaskTrackerVisible = false;
}

void UDBAGameUIManager::ToggleNewbieTaskTracker()
{
	bNewbieTaskTrackerVisible ? HideNewbieTaskTracker() : ShowNewbieTaskTracker();
}

void UDBAGameUIManager::ShowLobbyPlayerHUD()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (LobbyPlayerHUDWidget)
	{
		const UWorld* CurrentWorld = World;
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
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 大厅玩家界面已添加到视口：%s"),
			*LobbyPlayerHUDWidget->GetClass()->GetName());
	}
	else if (LobbyPlayerHUDWidget && bLobbyPlayerHUDVisible)
	{
		LobbyPlayerHUDWidget->RefreshFromCurrentCharacterData();
	}
	else if (!LobbyPlayerHUDWidget && IsLobbyGameplayWorldForUIManager(World))
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
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!MainLobbyWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		MainLobbyWidget = CreateWidget<UDBAMainLobbyWidgetBase>(PC, MainLobbyWidgetClass);
	}
}

void UDBAGameUIManager::CreateArenaHUDWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!ArenaHUDWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		ArenaHUDWidget = CreateWidget<UDBAArenaHUDRootWidgetBase>(PC, ArenaHUDWidgetClass);
		if (ArenaHUDWidget)
		{
			ArenaHUDWidgetController = EnsureArenaHUDWidgetController(PC);
			ArenaHUDWidget->SetWidgetController(ArenaHUDWidgetController);
			ArenaHUDWidget->BindArenaHUDToCharacter(ArenaHUDCharacter.Get());
		}
	}
}

APlayerController* UDBAGameUIManager::GetArenaHUDLocalPlayerController() const
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return nullptr;
	}

	return World->GetFirstPlayerController();
}

UDBAArenaHUDWidgetController* UDBAGameUIManager::EnsureArenaHUDWidgetController(APlayerController* InPlayerController)
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return nullptr;
	}

	if (!InPlayerController || InPlayerController->GetWorld() != World)
	{
		return nullptr;
	}

	if (!ArenaHUDWidgetController)
	{
		ArenaHUDWidgetController = NewObject<UDBAArenaHUDWidgetController>(this);
	}

	if (ArenaHUDWidgetController)
	{
		ArenaHUDWidgetController->InitializeController(InPlayerController);
	}

	return ArenaHUDWidgetController;
}

void UDBAGameUIManager::CreateGameSettingsWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!GameSettingsWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		GameSettingsWidget = CreateWidget<UDBAGameSettingsWidgetBase>(PC, GameSettingsWidgetClass);
	}
}

void UDBAGameUIManager::CreateInventoryWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!InventoryWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		InventoryWidget = CreateWidget<UDBAInventoryWidgetBase>(PC, InventoryWidgetClass);
	}
}

void UDBAGameUIManager::CreatePartyPanelWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!PartyPanelWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		PartyPanelWidget = CreateWidget<UDBAPartyPanelWidgetBase>(PC, PartyPanelWidgetClass);
	}
}

void UDBAGameUIManager::CreateInvitePanelWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!InvitePanelWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		InvitePanelWidget = CreateWidget<UDBAInvitePanelWidgetBase>(PC, InvitePanelWidgetClass);
	}
}

void UDBAGameUIManager::CreateQueueModeSelectWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!QueueModeSelectWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		QueueModeSelectWidget = CreateWidget<UDBAQueueModeSelectWidgetBase>(PC, QueueModeSelectWidgetClass);
	}
}

void UDBAGameUIManager::CreateQueueStatusWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!QueueStatusWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		QueueStatusWidget = CreateWidget<UDBAQueueStatusWidgetBase>(PC, QueueStatusWidgetClass);
	}
}

void UDBAGameUIManager::CreateReadyCheckWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!ReadyCheckWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		ReadyCheckWidget = CreateWidget<UDBAReadyCheckWidgetBase>(PC, ReadyCheckWidgetClass);
	}
}

void UDBAGameUIManager::CreateMatchFoundWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!MatchFoundWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		MatchFoundWidget = CreateWidget<UDBAMatchFoundWidgetBase>(PC, MatchFoundWidgetClass);
	}
}

void UDBAGameUIManager::CreatePortalConfirmWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!PortalConfirmWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		PortalConfirmWidget = CreateWidget<UDBAPortalConfirmWidgetBase>(PC, PortalConfirmWidgetClass);
	}
}

void UDBAGameUIManager::CreateInteractionPromptWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!InteractionPromptWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		InteractionPromptWidget = CreateWidget<UDBAInteractionPromptWidgetBase>(PC, InteractionPromptWidgetClass);
	}
}

void UDBAGameUIManager::CreateNewbieVillageMainWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!NewbieVillageMainWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		NewbieVillageMainWidget = CreateWidget<UDBANewbieVillageMainWidgetBase>(PC, NewbieVillageMainWidgetClass);
	}
}

void UDBAGameUIManager::CreateNewbieTaskTrackerWidget()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!NewbieTaskTrackerWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		NewbieTaskTrackerWidget = CreateWidget<UDBANewbieTaskTrackerWidgetBase>(PC, NewbieTaskTrackerWidgetClass);
	}
}

void UDBAGameUIManager::HandleLoginFlowStateChanged(EDBALoginFlowState NewState)
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 登录流程状态已变化：%d"), static_cast<int32>(NewState));
	CachedLoginFlowState = NewState;
	RefreshLoginFlowWidgetVisibility();

	if (NewState == EDBALoginFlowState::CharacterSelecting || NewState == EDBALoginFlowState::CharacterCreating)
	{
		if (UDBAFrontendEnvironmentSubsystem* FrontendEnvironment = World->GetSubsystem<UDBAFrontendEnvironmentSubsystem>())
		{
			FrontendEnvironment->EnableCharacterPresentationRendering();
		}
	}

	if (NewState == EDBALoginFlowState::InVillage && IsLobbyGameplayWorldForUIManager(World))
	{
		TransitionTo(EDBAUIState::Lobby);
	}
}

void UDBAGameUIManager::RefreshLoginFlowWidgetVisibility()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	switch (CachedLoginFlowState)
	{
	case EDBALoginFlowState::Booting:
	{
		// Keep startup clean so splash video can appear before login flow widgets.
		HideMainLobby();
		HideAllFlowWidgets();
		ResetFlowWidgetRefreshRetry();
		break;
	}
	case EDBALoginFlowState::AwaitingLogin:
	case EDBALoginFlowState::Authenticating:
	case EDBALoginFlowState::LoadingCharacters:
	{
		// 登录失败或列表失败回退时，必须关闭选角/创建留下的加载遮罩。
		if (CachedLoginFlowState == EDBALoginFlowState::AwaitingLogin)
		{
			HideLobbyLoadingScreen();
		}
		StopLoginFlowBackgroundMusic();
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
	case EDBALoginFlowState::CharacterSelecting:
	{
		if (!HasCompletedAccountLogin(GetGameInstance()))
		{
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 未完成登录，拒绝显示角色选择界面并返回登录页。"));
			if (UDBAFrontendFlowSubsystem* LoginFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr)
			{
				LoginFlow->StartLoginFlow();
			}
			return;
		}
		HideLobbyLoadingScreen();
		StopLoginFlowBackgroundMusic();
		HideMainLobby();
		if (UDBACharacterSelectFlowWidgetBase* SelectWidget = EnsureFlowWidgetCreated(CharacterSelectWidgetClass, CharacterSelectWidget))
		{
			SetFlowWidgetVisible(SelectWidget);
			SelectWidget->ApplyCharacterFlowViewportPresentation();
			if (UDBAFrontendFlowSubsystem* LoginFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr)
			{
				SelectWidget->UpdateCharacters(LoginFlow->GetCachedCharacters());
			}
			RemoveAllViewportLoginWidgets(GetWorld());
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 已显示角色选择界面。"));
			ResetFlowWidgetRefreshRetry();
		}
		else
		{
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 角色选择控件尚未就绪，准备重试。"));
			ScheduleFlowWidgetRefreshRetry();
		}
		break;
	}
	case EDBALoginFlowState::CharacterCreating:
	{
		if (!HasCompletedAccountLogin(GetGameInstance()))
		{
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 未完成登录，拒绝显示角色创建界面并返回登录页。"));
			if (UDBAFrontendFlowSubsystem* LoginFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr)
			{
				LoginFlow->StartLoginFlow();
			}
			return;
		}
		HideLobbyLoadingScreen();
		StopLoginFlowBackgroundMusic();
		HideMainLobby();
		if (UDBACharacterCreateFlowWidgetBase* CreateWidget = EnsureFlowWidgetCreated(CharacterCreateWidgetClass, CharacterCreateWidget))
		{
			SetFlowWidgetVisible(CreateWidget);
			CreateWidget->ApplyCharacterFlowViewportPresentation();
			RemoveAllViewportLoginWidgets(GetWorld());
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 已显示角色创建界面。"));
			ResetFlowWidgetRefreshRetry();
		}
		else
		{
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 角色创建控件尚未就绪，准备重试。"));
			ScheduleFlowWidgetRefreshRetry();
		}
		break;
	}
	case EDBALoginFlowState::AllocatingVillage:
	case EDBALoginFlowState::WaitingVillageServer:
	case EDBALoginFlowState::ConnectingVillage:
	case EDBALoginFlowState::InitializingVillage:
	{
		ShowLobbyLoadingScreen();
		break;
	}
	case EDBALoginFlowState::InVillage:
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

void UDBAGameUIManager::RestoreInputModeAfterOverlayClosed()
{
	if (bIsDeinitializing)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	UUserWidget* FocusWidget = nullptr;
	if (ReadyCheckWidget && bReadyCheckVisible)
	{
		FocusWidget = ReadyCheckWidget;
	}
	else if (PortalConfirmWidget && bPortalConfirmVisible)
	{
		FocusWidget = PortalConfirmWidget;
	}
	else if (MatchFoundWidget && bMatchFoundVisible)
	{
		FocusWidget = MatchFoundWidget;
	}
	else if (QueueModeSelectWidget && bQueueModeSelectVisible)
	{
		FocusWidget = QueueModeSelectWidget;
	}
	else if (InvitePanelWidget && bInvitePanelVisible)
	{
		FocusWidget = InvitePanelWidget;
	}
	else if (PartyPanelWidget && bPartyPanelVisible)
	{
		FocusWidget = PartyPanelWidget;
	}
	else if (NewbieVillageMainWidget && bNewbieVillageMainVisible)
	{
		FocusWidget = NewbieVillageMainWidget;
	}
	else if (InventoryWidget && bInventoryVisible)
	{
		FocusWidget = InventoryWidget;
	}
	else if (GameSettingsWidget && bGameSettingsVisible)
	{
		FocusWidget = GameSettingsWidget;
	}

	if (FocusWidget)
	{
		ApplyFrontendInputMode(World, FocusWidget);
	}
	else
	{
		ApplyLobbyGameplayInputMode(World);
	}
}

void UDBAGameUIManager::HandleReadyCheckCompleted(bool bAccepted)
{
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 准备确认已完成：%s"), bAccepted ? TEXT("已接受") : TEXT("已拒绝"));
	HideReadyCheck();
}

void UDBAGameUIManager::HandlePortalConfirmed(FName DestinationId)
{
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 传送门已确认：%s"), *DestinationId.ToString());
	HidePortalConfirm();
}

void UDBAGameUIManager::HandlePortalCancelled()
{
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 传送门已取消。"));
	HidePortalConfirm();
}

template<typename WidgetType>
WidgetType* UDBAGameUIManager::EnsureFlowWidgetCreated(TSubclassOf<WidgetType> WidgetClass, TObjectPtr<WidgetType>& WidgetInstance)
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return nullptr;
	}

	if (WidgetInstance)
	{
		const UWorld* WidgetWorld = WidgetInstance->GetWorld();
		const APlayerController* OwningPC = WidgetInstance->GetOwningPlayer();
		const UWorld* OwningPlayerWorld = OwningPC ? OwningPC->GetWorld() : nullptr;
		const bool bWidgetWorldMatches = WidgetWorld == World || OwningPlayerWorld == World;
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
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC && World->GetGameInstance())
	{
		PC = World->GetGameInstance()->GetPrimaryPlayerController();
	}
	if (PC)
	{
		WidgetInstance = CreateWidget<WidgetType>(PC, WidgetClass);
	}
	return WidgetInstance;
}

void UDBAGameUIManager::ScheduleFlowWidgetRefreshRetry()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
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
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
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
		UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] 大厅界面重试超过上限。"));
		return;
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 大厅界面尚未就绪，准备第 %d 次重试。"), LobbyHUDRefreshRetryCount);
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
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!LobbyPlayerHUDWidgetClass)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		LobbyPlayerHUDWidget = CreateWidget<UDBALobbyPlayerHUDWidgetBase>(PC, LobbyPlayerHUDWidgetClass);
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 大厅玩家界面控件已创建：类=%s 成功=%s"),
			*LobbyPlayerHUDWidgetClass->GetName(),
			LobbyPlayerHUDWidget ? TEXT("是") : TEXT("否"));
	}
	else
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 大厅玩家界面正在等待玩家控制器。"));
	}
}

void UDBAGameUIManager::HandleFlowWidgetRefreshRetry()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	RefreshLoginFlowWidgetVisibility();
}

void UDBAGameUIManager::SetFlowWidgetVisible(UUserWidget* WidgetToShow)
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (!WidgetToShow)
	{
		return;
	}
	if (WidgetToShow->IsInViewport())
	{
		WidgetToShow->SetIsEnabled(true);
		WidgetToShow->SetVisibility(ESlateVisibility::Visible);
		if (UDBALoginFlowWidgetBase* LoginFlowWidget = Cast<UDBALoginFlowWidgetBase>(WidgetToShow))
		{
			LoginFlowWidget->ApplyLoginViewportPresentation();
			ApplyLoginFrontendInputMode(World, LoginFlowWidget);
		}
		bFlowWidgetVisible = true;
		return;
	}

	HideAllFlowWidgets();
	WidgetToShow->SetIsEnabled(true);
	WidgetToShow->SetVisibility(ESlateVisibility::Visible);
	WidgetToShow->AddToViewport(10);
	if (UDBALoginFlowWidgetBase* LoginFlowWidget = Cast<UDBALoginFlowWidgetBase>(WidgetToShow))
	{
		LoginFlowWidget->ApplyLoginViewportPresentation();
		LoginFlowWidget->ScheduleLoginLayoutRefresh();
		ApplyLoginFrontendInputMode(World, LoginFlowWidget);
	}
	else if (UDBACharacterSelectFlowWidgetBase* SelectFlowWidget = Cast<UDBACharacterSelectFlowWidgetBase>(WidgetToShow))
	{
		SelectFlowWidget->ApplyCharacterFlowViewportPresentation();
		ApplyFrontendInputMode(World, SelectFlowWidget);
	}
	else if (UDBACharacterCreateFlowWidgetBase* CreateFlowWidget = Cast<UDBACharacterCreateFlowWidgetBase>(WidgetToShow))
	{
		CreateFlowWidget->ApplyCharacterFlowViewportPresentation();
		ApplyFrontendInputMode(World, CreateFlowWidget);
	}
	else
	{
		DBAUIFonts::ApplyFullscreenFlowViewportPresentation(WidgetToShow);
		ApplyFrontendInputMode(World, WidgetToShow);
	}
	bFlowWidgetVisible = true;
}

void UDBAGameUIManager::ShowSplashVideo()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 请求显示启动视频，控件类=%s"), SplashVideoWidgetClass ? TEXT("有效") : TEXT("空"));

	if (!SplashVideoWidget)
	{
		if (!SplashVideoWidgetClass)
		{
			UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] 启动视频控件类为空，直接进入登录流程。"));
			EnsureLoginFlowStartedFromManager();
			return;
		}
		if (World)
		{
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 世界=%s 关卡地址=%s 玩家控制器数量=%d"), *World->GetName(), *World->URL.ToString(), World->GetNumPlayerControllers());
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
				UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 世界 %s 中没有找到玩家控制器，当前数量=%d，直接进入登录流程。"), *World->GetName(), World->GetNumPlayerControllers());
				EnsureLoginFlowStartedFromManager();
				return;
			}
		}
		else
		{
			UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] 没有找到世界对象，直接进入登录流程。"));
			EnsureLoginFlowStartedFromManager();
			return;
		}
	}
	if (SplashVideoWidget)
	{
		RemoveAllViewportLoginWidgets(World);
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
		// 启动视频必须全屏铺满，不能使用带 RenderScale 的缩放呈现（小窗口会只显示一角）。
		SplashVideoWidget->ApplySplashFullscreenPresentation();
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 启动视频控件已添加到视口并铺满全屏。"));

		// 设置键盘焦点到启动视频控件，以便接收 ESC 按键
		ApplySplashInputMode(World, SplashVideoWidget);
	}
	else
	{
		UE_LOG(LogDBACore, Error, TEXT("[DBAGameUIManager] 启动视频控件创建失败，直接进入登录流程。"));
		EnsureLoginFlowStartedFromManager();
	}
}

void UDBAGameUIManager::HideSplashVideo()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	if (SplashVideoWidget && SplashVideoWidget->IsInViewport())
	{
		SplashVideoWidget->RemoveFromParent();
	}
}

void UDBAGameUIManager::EnsureLoginFlowBackgroundMusic()
{
	UWorld* World = GetWorld();
	if (!IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		return;
	}

	const UDBAUIDeveloperSettings* UISettings = GetDefault<UDBAUIDeveloperSettings>();
	if (!UISettings)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 未找到 UI 音乐配置，无法播放流程背景音乐。"));
		return;
	}

	TSoftObjectPtr<USoundBase> DesiredMusic;
	switch (CachedLoginFlowState)
	{
	case EDBALoginFlowState::CharacterCreating:
		DesiredMusic = UISettings->CharacterCreateBGM;
		break;
	case EDBALoginFlowState::CharacterSelecting:
		DesiredMusic = UISettings->CharacterSelectBGM;
		break;
	default:
		DesiredMusic = UISettings->LoginFlowBGM;
		break;
	}

	const FSoftObjectPath DesiredMusicPath = DesiredMusic.ToSoftObjectPath();
	if (!DesiredMusicPath.IsValid())
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 登录流程背景音乐软引用路径无效。"));
		return;
	}

	if (LoginFlowBackgroundMusicComponent && LoginFlowBackgroundMusicSound
		&& LoginFlowBackgroundMusicSound->GetPathName() == DesiredMusicPath.ToString())
	{
		return;
	}
	if (PendingLoginFlowBackgroundMusicPath == DesiredMusicPath)
	{
		return;
	}

	StopLoginFlowBackgroundMusic();
	PendingLoginFlowBackgroundMusicPath = DesiredMusicPath;

	if (USoundBase* LoadedSound = DesiredMusic.Get())
	{
		PendingLoginFlowBackgroundMusicPath.Reset();
		StartLoginFlowBackgroundMusic(LoadedSound);
		return;
	}

	LoginFlowBackgroundMusicStreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		DesiredMusicPath,
		FStreamableDelegate::CreateWeakLambda(this, [this, DesiredMusicPath]()
		{
			LoginFlowBackgroundMusicStreamableHandle.Reset();
			if (PendingLoginFlowBackgroundMusicPath != DesiredMusicPath)
			{
				return;
			}

			PendingLoginFlowBackgroundMusicPath.Reset();
			TSoftObjectPtr<USoundBase> LoadedMusic(DesiredMusicPath);
			if (USoundBase* LoadedSound = LoadedMusic.Get())
			{
				StartLoginFlowBackgroundMusic(LoadedSound);
			}
			else
			{
				UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 登录流程背景音乐异步加载完成但资源为空：%s。"), *DesiredMusicPath.ToString());
			}
		}),
		FStreamableManager::AsyncLoadHighPriority,
		true);

	if (!LoginFlowBackgroundMusicStreamableHandle.IsValid())
	{
		PendingLoginFlowBackgroundMusicPath.Reset();
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 无法启动登录流程背景音乐异步加载：%s。"), *DesiredMusicPath.ToString());
	}
}

void UDBAGameUIManager::StartLoginFlowBackgroundMusic(USoundBase* DesiredSound)
{
	UWorld* World = GetWorld();
	if (!DesiredSound || !IsWorldSafeForWidgetCreation(World) || IsServerLikeRuntime(World))
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 登录流程背景音乐资源为空，无法播放。"));
		return;
	}

	LoginFlowBackgroundMusicSound = DesiredSound;
	LoginFlowBackgroundMusicComponent = UGameplayStatics::SpawnSound2D(World, LoginFlowBackgroundMusicSound, 0.72f, 1.0f, 0.0f, nullptr, false, false);
	if (!LoginFlowBackgroundMusicComponent)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] 启动登录流程背景音乐失败。"));
		return;
	}

	LoginFlowBackgroundMusicComponent->bIsUISound = true;
	LoginFlowBackgroundMusicComponent->OnAudioFinished.RemoveDynamic(this, &UDBAGameUIManager::HandleLoginFlowBackgroundMusicFinished);
	LoginFlowBackgroundMusicComponent->OnAudioFinished.AddDynamic(this, &UDBAGameUIManager::HandleLoginFlowBackgroundMusicFinished);
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] 已播放流程背景音乐：状态=%d，资源=%s。"),
		static_cast<int32>(CachedLoginFlowState),
		*LoginFlowBackgroundMusicSound->GetPathName());
}

void UDBAGameUIManager::StopLoginFlowBackgroundMusic()
{
	LoginFlowBackgroundMusicStreamableHandle.Reset();
	PendingLoginFlowBackgroundMusicPath.Reset();
	LoginFlowBackgroundMusicSound = nullptr;

	if (!LoginFlowBackgroundMusicComponent)
	{
		return;
	}

	LoginFlowBackgroundMusicComponent->OnAudioFinished.RemoveDynamic(this, &UDBAGameUIManager::HandleLoginFlowBackgroundMusicFinished);
	LoginFlowBackgroundMusicComponent->Stop();
	LoginFlowBackgroundMusicComponent = nullptr;
	LoginFlowBackgroundMusicSound = nullptr;
}

void UDBAGameUIManager::HandleLoginFlowBackgroundMusicFinished()
{
	LoginFlowBackgroundMusicComponent = nullptr;
	EnsureLoginFlowBackgroundMusic();
}
