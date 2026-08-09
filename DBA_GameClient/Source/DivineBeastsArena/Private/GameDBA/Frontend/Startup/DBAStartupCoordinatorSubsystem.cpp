// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Startup/DBAStartupCoordinatorSubsystem.h"

#include "Engine/GameInstance.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendConfigService.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameDBA/Frontend/Settings/DBAFrontendSettings.h"
#include "GameDBA/Frontend/Startup/DBAStartupPolicy.h"
#include "GameDBA/UI/Controllers/DBAGameUIManager.h"
#include "GameDBA/UI/Subsystems/DBAUILayerManagerSubsystem.h"
#include "GameDBA/UI/Widgets/Startup/UDBAStartupVideoWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "TimerManager.h"

namespace
{
	bool IsFrontendWorld(const UWorld* World, const FName FrontendMapPath)
	{
		return World && World->PersistentLevel && !FrontendMapPath.IsNone()
			&& World->PersistentLevel->GetOutermost()->GetFName() == FrontendMapPath;
	}
}

void UDBAStartupCoordinatorSubsystem::OnSubsystemDeinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BackendCheckTimeoutHandle);
	}
	if (StartupScreen)
	{
		StartupScreen->OnContinueRequested.RemoveDynamic(this, &UDBAStartupCoordinatorSubsystem::HandleStartupContinueRequested);
	}
	StartupScreen = nullptr;
	ViewModel = nullptr;
	Super::OnSubsystemDeinitialize();
}

bool UDBAStartupCoordinatorSubsystem::IsClientRuntime() const
{
	const UWorld* World = GetWorld();
	return World && !IsRunningDedicatedServer() && World->GetNetMode() != NM_DedicatedServer;
}

void UDBAStartupCoordinatorSubsystem::SetPhase(EDBAStartupPhase NewPhase)
{
	StartupPhase = NewPhase;
	UE_LOG(LogDBAFrontend, Log, TEXT("启动编排阶段切换为 %d。"), static_cast<int32>(NewPhase));
}

bool UDBAStartupCoordinatorSubsystem::ValidateConfiguration(FName& OutFrontendMapPath) const
{
	const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
	if (!Settings)
	{
		return false;
	}
	const FSoftObjectPath MapPath = Settings->FrontendMap.ToSoftObjectPath();
	if (!DBAStartupPolicy::IsFrontendMapConfigurationValid(MapPath))
	{
		return false;
	}
	OutFrontendMapPath = FName(*MapPath.GetLongPackageName());
	return !OutFrontendMapPath.IsNone();
}

void UDBAStartupCoordinatorSubsystem::BeginStartup()
{
	if (!IsClientRuntime() || StartupPhase != EDBAStartupPhase::NotStarted)
	{
		return;
	}
	ViewModel = NewObject<UDBAStartupViewModel>(this);
	const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
	const FText Title = Settings && !Settings->StartupTitle.IsEmpty()
		? Settings->StartupTitle
		: NSLOCTEXT("DBAStartup", "DefaultTitle", "神兽竞技场");
	ViewModel->SetPresentation(Title, FText::FromString(FApp::GetBuildVersion()));

	SetPhase(EDBAStartupPhase::ReadingConfiguration);
	if (!ValidateConfiguration(FrontendMapPath))
	{
		EnterFatalFailure(NSLOCTEXT("DBAStartup", "MissingFrontendMap", "前台地图配置缺失或无效，请联系技术支持。"));
		return;
	}
	LoadLocalPreferences();
	PrepareSecureSession();
	StartBackendCheck();
}

void UDBAStartupCoordinatorSubsystem::LoadLocalPreferences()
{
	SetPhase(EDBAStartupPhase::LoadingLocalPreferences);
	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UserSettings->LoadSettings(false);
	}
}

void UDBAStartupCoordinatorSubsystem::PrepareSecureSession()
{
	// 账户服务独占凭据读写；此处仅保证其生命周期已经准备完成，绝不读取或记录 Token。
	SetPhase(EDBAStartupPhase::PreparingSecureSession);
	if (!GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>())
	{
		EnterFatalFailure(NSLOCTEXT("DBAStartup", "MissingFrontendFlow", "前台流程服务不可用，无法继续启动。"));
	}
}

void UDBAStartupCoordinatorSubsystem::StartBackendCheck()
{
	if (StartupPhase == EDBAStartupPhase::FatalFailure)
	{
		return;
	}
	const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
	if (!Settings || !Settings->bCheckBackendVersionOnStartup)
	{
		CompleteBackendCheck(true, NSLOCTEXT("DBAStartup", "ServiceCheckSkipped", "服务状态将在登录时验证。"));
		return;
	}

	SetPhase(EDBAStartupPhase::CheckingBackend);
	ViewModel->SetServiceStatus(EDBAStartupServiceState::Checking, NSLOCTEXT("DBAStartup", "CheckingService", "正在检查服务状态…"), false);
	bBackendCheckResolved = false;
	UDBA_GameBackendClientSubsystem* Backend = GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>();
	UDBA_GameBackendConfigService* ConfigService = Backend ? Backend->GetConfigService() : nullptr;
	if (!ConfigService)
	{
		CompleteBackendCheck(false, NSLOCTEXT("DBAStartup", "ServiceUnavailable", "服务暂时不可用，可离线进入登录页。"));
		return;
	}

	FDBA_GameBackendResponseDelegate Callback;
	Callback.BindDynamic(this, &UDBAStartupCoordinatorSubsystem::HandleBackendVersionCheck);
	ConfigService->VersionCheck(Callback);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(BackendCheckTimeoutHandle, this, &UDBAStartupCoordinatorSubsystem::HandleBackendCheckTimeout, Settings->StartupReachabilityTimeoutSeconds, false);
	}
}

void UDBAStartupCoordinatorSubsystem::HandleBackendVersionCheck(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bBackendCheckResolved)
	{
		CompleteBackendCheck(bSuccess, bSuccess
			? NSLOCTEXT("DBAStartup", "ServiceOnline", "服务连接正常。")
			: NSLOCTEXT("DBAStartup", "ServiceOffline", "服务暂时不可用，可离线进入登录页。"));
	}
}

void UDBAStartupCoordinatorSubsystem::HandleBackendCheckTimeout()
{
	if (!bBackendCheckResolved)
	{
		CompleteBackendCheck(false, NSLOCTEXT("DBAStartup", "ServiceTimeout", "服务检查超时，可离线进入登录页。"));
	}
}

void UDBAStartupCoordinatorSubsystem::CompleteBackendCheck(bool bOnline, const FText& StatusText)
{
	if (bBackendCheckResolved)
	{
		return;
	}
	bBackendCheckResolved = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BackendCheckTimeoutHandle);
	}
	ViewModel->SetServiceStatus(bOnline ? EDBAStartupServiceState::Online : EDBAStartupServiceState::OfflineRecoverable, StatusText, true);
	const EDBAStartupCheckDisposition Disposition = DBAStartupPolicy::ResolveBackendCheck(!FrontendMapPath.IsNone(), bOnline);
	if (Disposition == EDBAStartupCheckDisposition::FatalFailure)
	{
		EnterFatalFailure(NSLOCTEXT("DBAStartup", "InvalidFrontendMap", "前台地图配置无效，无法继续启动。"));
		return;
	}
	SetPhase(bOnline ? EDBAStartupPhase::TravellingToFrontend : EDBAStartupPhase::RecoverableFailure);
	BeginFrontendTravel();
}

void UDBAStartupCoordinatorSubsystem::BeginFrontendTravel()
{
	if (bFrontendTravelRequested || FrontendMapPath.IsNone())
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		EnterFatalFailure(NSLOCTEXT("DBAStartup", "MissingWorld", "启动世界不可用，无法进入前台。"));
		return;
	}
	bFrontendTravelRequested = true;
	if (IsFrontendWorld(World, FrontendMapPath))
	{
		PresentStartupScreen();
		return;
	}
	SetPhase(EDBAStartupPhase::TravellingToFrontend);
	UE_LOG(LogDBAFrontend, Log, TEXT("启动检查完成，切换到持久前台地图：%s。"), *FrontendMapPath.ToString());
	UGameplayStatics::OpenLevel(World, FrontendMapPath);
}

void UDBAStartupCoordinatorSubsystem::HandleWorldChanged(UWorld* NewWorld)
{
	if (IsClientRuntime() && bFrontendTravelRequested && IsFrontendWorld(NewWorld, FrontendMapPath))
	{
		PresentStartupScreen();
	}
}

void UDBAStartupCoordinatorSubsystem::PresentStartupScreen()
{
	if (!ViewModel)
	{
		return;
	}
	const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
	if (!Settings || Settings->StartupScreenWidgetClass.IsNull())
	{
		if (StartupPhase != EDBAStartupPhase::FatalFailure)
		{
			EnterFatalFailure(NSLOCTEXT("DBAStartup", "MissingStartupScreen", "启动页配置缺失，无法安全进入前台。"));
		}
		return;
	}

	UClass* LoadedClass = Settings->StartupScreenWidgetClass.LoadSynchronous();
	UDBAUILayerManagerSubsystem* Layers = GetGameInstance()->GetSubsystem<UDBAUILayerManagerSubsystem>();
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!LoadedClass || !Layers || !PlayerController)
	{
		if (StartupPhase != EDBAStartupPhase::FatalFailure)
		{
			EnterFatalFailure(NSLOCTEXT("DBAStartup", "StartupScreenUnavailable", "启动页无法创建，无法安全进入前台。"));
		}
		return;
	}
	if (!StartupScreen)
	{
		StartupScreen = CreateWidget<UDBAStartupVideoWidget>(PlayerController, LoadedClass);
		if (StartupScreen)
		{
			StartupScreen->OnContinueRequested.AddDynamic(this, &UDBAStartupCoordinatorSubsystem::HandleStartupContinueRequested);
		}
	}
	if (!StartupScreen || !Layers->MountWidget(StartupScreen, EDBAUILayer::Screen))
	{
		if (StartupPhase != EDBAStartupPhase::FatalFailure)
		{
			EnterFatalFailure(NSLOCTEXT("DBAStartup", "StartupScreenMountFailed", "启动页无法显示，无法安全进入前台。"));
		}
		return;
	}
	StartupScreen->SetStartupViewModel(ViewModel);
	if (StartupPhase != EDBAStartupPhase::FatalFailure)
	{
		SetPhase(EDBAStartupPhase::AwaitingContinue);
	}
}

void UDBAStartupCoordinatorSubsystem::HandleStartupContinueRequested()
{
	if (StartupPhase == EDBAStartupPhase::AwaitingContinue && ViewModel && ViewModel->bCanContinue)
	{
		FinishStartupAndEnterFlow();
	}
}

void UDBAStartupCoordinatorSubsystem::FinishStartupAndEnterFlow()
{
	if (UDBAUILayerManagerSubsystem* Layers = GetGameInstance()->GetSubsystem<UDBAUILayerManagerSubsystem>())
	{
		Layers->RemoveWidget(StartupScreen);
	}
	StartupScreen = nullptr;
	SetPhase(EDBAStartupPhase::Completed);
	if (UDBAFrontendFlowSubsystem* Flow = GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>())
	{
		Flow->StartLoginFlow();
	}
	if (UDBAGameUIManager* UIManager = GetGameInstance()->GetSubsystem<UDBAGameUIManager>())
	{
		UIManager->RequestShowLoginFlowWidget();
	}
}

void UDBAStartupCoordinatorSubsystem::EnterFatalFailure(const FText& FailureText)
{
	SetPhase(EDBAStartupPhase::FatalFailure);
	if (!ViewModel)
	{
		ViewModel = NewObject<UDBAStartupViewModel>(this);
	}
	ViewModel->SetServiceStatus(EDBAStartupServiceState::FatalConfiguration, FailureText, false);
	UE_LOG(LogDBAFrontend, Error, TEXT("启动失败：%s"), *FailureText.ToString());
	if (IsClientRuntime())
	{
		PresentStartupScreen();
	}
}

void UDBAStartupCoordinatorSubsystem::RetryBackendCheck()
{
	if (StartupPhase == EDBAStartupPhase::RecoverableFailure || StartupPhase == EDBAStartupPhase::AwaitingContinue)
	{
		StartBackendCheck();
	}
}
