// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Framework/GameInstance/DBAGameInstance.h"

#include "Dom/JsonObject.h"
#include "GameCore/Networking/Account/DBAAccountServiceBase.h"
#include "GameCore/Session/Party/DBAPartyServiceBase.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendMatchService.h"
#include "GameBackendSessionService.h"
#include "GameBackendTelemetryService.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/Controllers/DBAGameUIManager.h"
#include "GameDBA/Frontend/DBAFrontendEnvironmentSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CommandLine.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"

namespace
{
	const FName FrontendMapPath(TEXT("/Game/Maps/Lobby/FrontendMap"));

	bool IsFrontendWorld(const UWorld* World)
	{
		if (!World || !World->PersistentLevel)
		{
			return false;
		}

		const FString LevelPath = World->PersistentLevel->GetOutermost()->GetName();
		return LevelPath.Contains(TEXT("FrontendMap"));
	}

	bool IsLobbyWorld(const UWorld* World)
	{
		if (!World || !World->PersistentLevel)
		{
			return false;
		}

		const FString LevelPath = World->PersistentLevel->GetOutermost()->GetName();
		return LevelPath.Contains(TEXT("LobbyMap"))
			|| LevelPath.Contains(TEXT("MainLobby"));
	}

	bool IsServerRuntime(const UWorld* World)
	{
		return IsRunningDedicatedServer()
			|| FParse::Param(FCommandLine::Get(), TEXT("server"))
			|| (World && World->GetNetMode() == NM_DedicatedServer);
	}

	FName GetConfiguredMapPath(const TCHAR* ConfigKey, const FName& FallbackPath)
	{
		FString ConfigValue;
		if (GConfig && GConfig->GetString(TEXT("/Script/DivineBeastsArena.DBAFrontendConfig"), ConfigKey, ConfigValue, GGameIni))
		{
			ConfigValue = ConfigValue.TrimStartAndEnd();
			if (!ConfigValue.IsEmpty())
			{
				return FName(*ConfigValue);
			}
		}
		return FallbackPath;
	}

}

UDBAGameInstance::UDBAGameInstance()
{
}

bool UDBAGameInstance::CanEnterLobbyGameplay(const UGameInstance* GameInstance)
{
	const UDBAFrontendFlowSubsystem* LoginFlow = GameInstance
		? GameInstance->GetSubsystem<UDBAFrontendFlowSubsystem>()
		: nullptr;
	return LoginFlow && LoginFlow->GetFlowState() == EDBALoginFlowState::InVillage;
}

void UDBAGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 游戏实例初始化完成。"));
}

void UDBAGameInstance::Shutdown()
{
	if (UDBA_GameBackendClientSubsystem* Backend = GetSubsystem<UDBA_GameBackendClientSubsystem>())
	{
		if (Backend->GetTelemetryService())
		{
			TMap<FString, FString> Props;
			Props.Add(TEXT("phase"), TEXT("game_shutdown"));
			Backend->GetTelemetryService()->TrackEvent(TEXT("client_exit"), Props);
			Backend->GetTelemetryService()->Flush();
		}
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 关闭游戏实例。"));
	Super::Shutdown();
}

void UDBAGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	Super::OnWorldChanged(OldWorld, NewWorld);

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 世界切换: %s -> %s"),
		OldWorld ? *OldWorld->GetName() : TEXT("无"),
		NewWorld ? *NewWorld->GetName() : TEXT("无"));

	if (IsDedicatedServerInstance() || IsServerRuntime(NewWorld))
	{
		return;
	}

	if (bPendingStartLoginFlowOnFrontend && IsFrontendWorld(NewWorld))
	{
		bPendingStartLoginFlowOnFrontend = false;
		StartLoginFlow();
	}

	UDBAFrontendFlowSubsystem* LoginFlow = GetSubsystem<UDBAFrontendFlowSubsystem>();
	UDBAGameUIManager* UIManager = GetSubsystem<UDBAGameUIManager>();
	const EDBALoginFlowState FlowState = LoginFlow ? LoginFlow->GetFlowState() : EDBALoginFlowState::Booting;

	if (IsFrontendWorld(NewWorld) && FlowState != EDBALoginFlowState::InVillage)
	{
		if (UDBAFrontendEnvironmentSubsystem* FrontendEnvironment = NewWorld->GetSubsystem<UDBAFrontendEnvironmentSubsystem>())
		{
			FrontendEnvironment->ApplyFrontendUiOnlyEnvironment();
		}

		if (UIManager)
		{
			UIManager->RequestShowLoginFlowWidget();
		}
		return;
	}

	if (IsLobbyWorld(NewWorld))
	{
		if (FlowState == EDBALoginFlowState::ConnectingVillage)
		{
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 已进入大厅服务器世界，等待本地 PlayerState 同步后显示大厅界面。"));
			return;
		}

		if (!CanEnterLobbyGameplay(this))
		{
			UE_LOG(LogDBACore, Warning, TEXT("[DBAGameInstance] 未完成前端登录/选角流程，返回前端地图。"));
			StartLoginFlow();
			return;
		}

		if (UIManager)
		{
			UIManager->ShowMainLobby();
		}
		return;
	}

	if (FlowState == EDBALoginFlowState::InVillage && IsFrontendWorld(NewWorld))
	{
		if (UIManager)
		{
			UIManager->RequestShowLoginFlowWidget();
		}
	}
}

void UDBAGameInstance::StartLoginFlow()
{
	if (IsDedicatedServerInstance() || IsServerRuntime(GetWorld()))
	{
		UE_LOG(LogDBACore, Verbose, TEXT("[DBAGameInstance] 服务器运行模式跳过前端登录流程。"));
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (!IsFrontendWorld(World))
		{
			bPendingStartLoginFlowOnFrontend = true;
			const FName ConfiguredFrontendMapPath = GetConfiguredMapPath(TEXT("DefaultFrontendMap"), FrontendMapPath);
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 登录流程前切换到前端地图: %s"), *ConfiguredFrontendMapPath.ToString());
			UGameplayStatics::OpenLevel(World, ConfiguredFrontendMapPath);
			return;
		}
	}

	if (bLoginFlowStarted)
	{
		if (UDBAFrontendFlowSubsystem* LoginFlow = GetSubsystem<UDBAFrontendFlowSubsystem>())
		{
			if (LoginFlow->GetFlowState() == EDBALoginFlowState::Booting)
			{
				LoginFlow->StartLoginFlow();
			}
		}
		if (UDBAGameUIManager* UIManager = GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->RequestShowLoginFlowWidget();
		}
		return;
	}

	bLoginFlowStarted = true;
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 启动登录流程。"));

	if (UDBAFrontendFlowSubsystem* LoginFlow = GetSubsystem<UDBAFrontendFlowSubsystem>())
	{
		LoginFlow->StartLoginFlow();
	}

	if (UDBAGameUIManager* UIManager = GetSubsystem<UDBAGameUIManager>())
	{
		UIManager->RequestShowLoginFlowWidget();
	}
}
