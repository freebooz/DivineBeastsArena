// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/DBAGameUIManager.h"
#include "GameDBA/UI/Splash/UDBASplashVideoWidget.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GameInstance/DBAGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "DBA_GameBackendClientSubsystem.h"
#include "DBA_GameBackendTelemetryService.h"
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
#include "GameDBA/UI/Lobby/UDBAInvitePanelWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAInteractionPromptWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAMatchFoundWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBANewbieTaskTrackerWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBANewbieVillageMainWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAPartyPanelWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAPortalConfirmWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAQueueModeSelectWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAQueueStatusWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAReadyCheckWidgetBase.h"
#include "GameDBA/UI/Common/UDBASoftwareCursorWidget.h"
#include "Components/AudioComponent.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	bool IsWorldSafeForWidgetCreation(const UWorld* World)
	{
		return World && !World->bIsTearingDown;
	}

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

	bool BlueprintClassPackageExists(const FString& ClassObjectPath)
	{
		const FSoftObjectPath SoftObjectPath(ClassObjectPath);
		const FString PackageName = SoftObjectPath.GetLongPackageName();
		return !PackageName.IsEmpty() && FPackageName::DoesPackageExist(PackageName);
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

			if (!BlueprintClassPackageExists(ClassObjectPath))
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

		Widget->SetAnchorsInViewport(Anchors);
		Widget->SetAlignmentInViewport(Alignment);
		Widget->SetPositionInViewport(Position, false);
		Widget->SetDesiredSizeInViewport(ClampWidgetSizeToViewport(Widget, DesiredSize, MinimumSize));
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

	PartyPanelWidgetClass = ResolveWidgetClassPath<UDBAPartyPanelWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/Party/WBP_DBA_PartyPanel"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_PartyPanel")
	});
	if (!PartyPanelWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Party panel widget blueprint is unavailable."));
	}

	InvitePanelWidgetClass = ResolveWidgetClassPath<UDBAInvitePanelWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/Party/WBP_DBA_InvitePanel"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_InvitePanel")
	});
	if (!InvitePanelWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Invite panel widget blueprint is unavailable."));
	}

	QueueModeSelectWidgetClass = ResolveWidgetClassPath<UDBAQueueModeSelectWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/Queue/WBP_DBA_QueueModeSelect"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_QueueModeSelect")
	});
	if (!QueueModeSelectWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Queue mode select widget blueprint is unavailable."));
	}

	QueueStatusWidgetClass = ResolveWidgetClassPath<UDBAQueueStatusWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/Queue/WBP_DBA_QueueStatus"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_QueueStatus")
	});
	if (!QueueStatusWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Queue status widget blueprint is unavailable."));
	}

	ReadyCheckWidgetClass = ResolveWidgetClassPath<UDBAReadyCheckWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/ReadyCheck/WBP_DBA_ReadyCheck"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_ReadyCheck")
	});
	if (!ReadyCheckWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Ready check widget blueprint is unavailable."));
	}

	MatchFoundWidgetClass = ResolveWidgetClassPath<UDBAMatchFoundWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/Queue/WBP_DBA_MatchFound"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_MatchFound")
	});
	if (!MatchFoundWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Match found widget blueprint is unavailable."));
	}

	PortalConfirmWidgetClass = ResolveWidgetClassPath<UDBAPortalConfirmWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/Portal/WBP_DBA_PortalConfirm"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_PortalConfirm")
	});
	if (!PortalConfirmWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Portal confirm widget blueprint is unavailable."));
	}

	InteractionPromptWidgetClass = ResolveWidgetClassPath<UDBAInteractionPromptWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/Interaction/WBP_DBA_InteractionPrompt"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_InteractionPrompt")
	});
	if (!InteractionPromptWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Interaction prompt widget blueprint is unavailable."));
	}

	NewbieVillageMainWidgetClass = ResolveWidgetClassPath<UDBANewbieVillageMainWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/NewbieVillage/WBP_DBA_NewbieVillageMain"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_NewbieVillageMain")
	});
	if (!NewbieVillageMainWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Newbie village main widget blueprint is unavailable."));
	}

	NewbieTaskTrackerWidgetClass = ResolveWidgetClassPath<UDBANewbieTaskTrackerWidgetBase>({
		TEXT("/Game/DBA/UI/Lobby/NewbieVillage/WBP_DBA_NewbieTaskTracker"),
		TEXT("/Game/Blueprints/UI/DBA/Lobby/WBP_DBA_NewbieTaskTracker")
	});
	if (!NewbieTaskTrackerWidgetClass)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameUIManager] Newbie task tracker widget blueprint is unavailable."));
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

	if (UDBALoginFlowSubsystem* LoginFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>() : nullptr)
	{
		LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBAGameUIManager::HandleLoginFlowStateChanged);
		LoginFlow->OnFlowStateChanged.AddDynamic(this, &UDBAGameUIManager::HandleLoginFlowStateChanged);
		CachedLoginFlowState = LoginFlow->GetFlowState();
		RefreshLoginFlowWidgetVisibility();
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
	bIsDeinitializing = true;

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
	if (InteractionPromptWidget && bInteractionPromptVisible)
	{
		InteractionPromptWidget->HidePrompt();
		InteractionPromptWidget->RemoveFromParent();
	}
	bInteractionPromptVisible = false;
}

void UDBAGameUIManager::UpdateInteractionProgress(float Progress)
{
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

void UDBAGameUIManager::CreatePartyPanelWidget()
{
	if (!PartyPanelWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			PartyPanelWidget = CreateWidget<UDBAPartyPanelWidgetBase>(PC, PartyPanelWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateInvitePanelWidget()
{
	if (!InvitePanelWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			InvitePanelWidget = CreateWidget<UDBAInvitePanelWidgetBase>(PC, InvitePanelWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateQueueModeSelectWidget()
{
	if (!QueueModeSelectWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			QueueModeSelectWidget = CreateWidget<UDBAQueueModeSelectWidgetBase>(PC, QueueModeSelectWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateQueueStatusWidget()
{
	if (!QueueStatusWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			QueueStatusWidget = CreateWidget<UDBAQueueStatusWidgetBase>(PC, QueueStatusWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateReadyCheckWidget()
{
	if (!ReadyCheckWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			ReadyCheckWidget = CreateWidget<UDBAReadyCheckWidgetBase>(PC, ReadyCheckWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateMatchFoundWidget()
{
	if (!MatchFoundWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			MatchFoundWidget = CreateWidget<UDBAMatchFoundWidgetBase>(PC, MatchFoundWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreatePortalConfirmWidget()
{
	if (!PortalConfirmWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			PortalConfirmWidget = CreateWidget<UDBAPortalConfirmWidgetBase>(PC, PortalConfirmWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateInteractionPromptWidget()
{
	if (!InteractionPromptWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			InteractionPromptWidget = CreateWidget<UDBAInteractionPromptWidgetBase>(PC, InteractionPromptWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateNewbieVillageMainWidget()
{
	if (!NewbieVillageMainWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			NewbieVillageMainWidget = CreateWidget<UDBANewbieVillageMainWidgetBase>(PC, NewbieVillageMainWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateNewbieTaskTrackerWidget()
{
	if (!NewbieTaskTrackerWidgetClass)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			NewbieTaskTrackerWidget = CreateWidget<UDBANewbieTaskTrackerWidgetBase>(PC, NewbieTaskTrackerWidgetClass);
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

void UDBAGameUIManager::RestoreInputModeAfterOverlayClosed()
{
	if (bIsDeinitializing)
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
		ApplyFrontendInputMode(GetWorld(), FocusWidget);
	}
	else
	{
		ApplyLobbyGameplayInputMode(GetWorld());
	}
}

void UDBAGameUIManager::HandleReadyCheckCompleted(bool bAccepted)
{
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] Ready check completed: %s"), bAccepted ? TEXT("accepted") : TEXT("declined"));
	HideReadyCheck();
}

void UDBAGameUIManager::HandlePortalConfirmed(FName DestinationId)
{
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] Portal confirmed: %s"), *DestinationId.ToString());
	HidePortalConfirm();
}

void UDBAGameUIManager::HandlePortalCancelled()
{
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameUIManager] Portal cancelled."));
	HidePortalConfirm();
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
