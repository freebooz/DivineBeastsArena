// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/GameInstance/DBAGameInstance.h"

#include "GameCore/Account/DBAAccountServiceBase.h"
#include "GameCore/Party/DBAPartyServiceBase.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendTelemetryService.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/DBAGameUIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CommandLine.h"

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

	FString GetCommandLineValueOrDefault(const TCHAR* Key, const FString& DefaultValue)
	{
		FString Value;
		if (FParse::Value(FCommandLine::Get(), Key, Value))
		{
			Value.TrimStartAndEndInline();
		}
		return Value.IsEmpty() ? DefaultValue : Value;
	}

	EDBAZodiac ParseZodiacName(const FString& Value)
	{
		struct FZodiacName
		{
			const TCHAR* Name;
			EDBAZodiac Zodiac;
		};

		const FZodiacName Names[] = {
			{ TEXT("Rat"), EDBAZodiac::Rat },
			{ TEXT("Ox"), EDBAZodiac::Ox },
			{ TEXT("Tiger"), EDBAZodiac::Tiger },
			{ TEXT("Rabbit"), EDBAZodiac::Rabbit },
			{ TEXT("Dragon"), EDBAZodiac::Dragon },
			{ TEXT("Snake"), EDBAZodiac::Snake },
			{ TEXT("Horse"), EDBAZodiac::Horse },
			{ TEXT("Goat"), EDBAZodiac::Goat },
			{ TEXT("Monkey"), EDBAZodiac::Monkey },
			{ TEXT("Rooster"), EDBAZodiac::Rooster },
			{ TEXT("Dog"), EDBAZodiac::Dog },
			{ TEXT("Pig"), EDBAZodiac::Pig }
		};

		for (const FZodiacName& Entry : Names)
		{
			if (Value.Equals(Entry.Name, ESearchCase::IgnoreCase)
				|| Value.EndsWith(FString(TEXT("_")) + Entry.Name, ESearchCase::IgnoreCase))
			{
				return Entry.Zodiac;
			}
		}

		return EDBAZodiac::None;
	}

	EDBAZodiac ResolveAutoLobbyZodiac(const FString& CharacterName)
	{
		const EDBAZodiac CommandLineZodiac = ParseZodiacName(GetCommandLineValueOrDefault(TEXT("DBAAutoZodiac="), TEXT("")));
		if (CommandLineZodiac != EDBAZodiac::None)
		{
			return CommandLineZodiac;
		}

		const EDBAZodiac NameZodiac = ParseZodiacName(CharacterName);
		return NameZodiac == EDBAZodiac::None ? EDBAZodiac::Rat : NameZodiac;
	}
}

UDBAGameInstance::UDBAGameInstance()
{
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
		OldWorld ? *OldWorld->GetName() : TEXT("None"),
		NewWorld ? *NewWorld->GetName() : TEXT("None"));

	if (IsDedicatedServerInstance() || IsServerRuntime(NewWorld))
	{
		return;
	}

	if (bPendingStartLoginFlowOnFrontend && IsFrontendWorld(NewWorld))
	{
		bPendingStartLoginFlowOnFrontend = false;
		StartLoginFlow();
	}

	UDBALoginFlowSubsystem* LoginFlow = GetSubsystem<UDBALoginFlowSubsystem>();
	UDBAGameUIManager* UIManager = GetSubsystem<UDBAGameUIManager>();
	const EDBALoginFlowState FlowState = LoginFlow ? LoginFlow->GetFlowState() : EDBALoginFlowState::Startup;

	if (IsFrontendWorld(NewWorld) && FlowState != EDBALoginFlowState::MainLobby)
	{
		if (UIManager)
		{
			UIManager->RequestShowLoginFlowWidget();
		}
		return;
	}

	if (IsLobbyWorld(NewWorld))
	{
		if (UIManager)
		{
			UIManager->ShowMainLobby();
		}
		RunAutoPartyStep();
		return;
	}

	if (FlowState == EDBALoginFlowState::MainLobby && IsFrontendWorld(NewWorld))
	{
		if (UIManager)
		{
			UIManager->RequestShowLoginFlowWidget();
		}
		RunAutoPartyStep();
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
		if (UDBALoginFlowSubsystem* LoginFlow = GetSubsystem<UDBALoginFlowSubsystem>())
		{
			if (LoginFlow->GetFlowState() == EDBALoginFlowState::Startup)
			{
				LoginFlow->StartLoginFlow();
			}
		}
		if (UDBAGameUIManager* UIManager = GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->RequestShowLoginFlowWidget();
		}
		TryStartAutoLobbyFlow();
		return;
	}

	bLoginFlowStarted = true;
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 启动登录流程。"));

	if (UDBALoginFlowSubsystem* LoginFlow = GetSubsystem<UDBALoginFlowSubsystem>())
	{
		LoginFlow->StartLoginFlow();
	}

	if (UDBAGameUIManager* UIManager = GetSubsystem<UDBAGameUIManager>())
	{
		UIManager->RequestShowLoginFlowWidget();
	}

	TryStartAutoLobbyFlow();
}

void UDBAGameInstance::HandleAutoLobbyFlowStateChanged(EDBALoginFlowState NewState)
{
	ContinueAutoLobbyFlow(NewState);
}

void UDBAGameInstance::TryStartAutoLobbyFlow()
{
	if (bAutoLobbyFlowStarted || !FParse::Param(FCommandLine::Get(), TEXT("DBAAutoLobbyFlow")))
	{
		return;
	}

	UDBALoginFlowSubsystem* LoginFlow = GetSubsystem<UDBALoginFlowSubsystem>();
	if (!LoginFlow)
	{
		return;
	}

	bAutoLobbyFlowStarted = true;
	LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBAGameInstance::HandleAutoLobbyFlowStateChanged);
	LoginFlow->OnFlowStateChanged.AddDynamic(this, &UDBAGameInstance::HandleAutoLobbyFlowStateChanged);
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 自动大厅流程已启用。"));
	ContinueAutoLobbyFlow(LoginFlow->GetFlowState());
}

void UDBAGameInstance::ContinueAutoLobbyFlow(EDBALoginFlowState FlowState)
{
	UDBALoginFlowSubsystem* LoginFlow = GetSubsystem<UDBALoginFlowSubsystem>();
	if (!LoginFlow)
	{
		return;
	}

	switch (FlowState)
	{
	case EDBALoginFlowState::LoginScreen:
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 自动大厅流程：提交访客登录。"));
		LoginFlow->SubmitGuestLogin();
		break;
	case EDBALoginFlowState::CharacterCreate:
	{
		FDBACharacterCreateRequest Request;
		Request.CharacterName = GetCommandLineValueOrDefault(TEXT("DBAAutoCharacterName="), TEXT("AutoLobbyRole"));
		Request.Zodiac = ResolveAutoLobbyZodiac(Request.CharacterName);
		Request.PrimaryElement = EDBAElement::Water;
		Request.FiveCamp = EDBAFiveCamp::East;
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 自动大厅流程：创建角色 %s zodiac=%d。"), *Request.CharacterName, static_cast<int32>(Request.Zodiac));
		LoginFlow->SubmitCharacterCreation(Request);
		break;
	}
	case EDBALoginFlowState::CharacterSelect:
	{
		const TArray<FDBACharacterSummary>& Characters = LoginFlow->GetCachedCharacters();
		if (Characters.Num() > 0)
		{
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 自动大厅流程：选择角色 %s。"), *Characters[0].CharacterName);
			LoginFlow->SubmitCharacterSelection(Characters[0].CharacterId);
		}
		break;
	}
	case EDBALoginFlowState::MainLobby:
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 自动大厅流程：已到达主大厅。"));
		RunAutoPartyStep();
		break;
	default:
		break;
	}
}

void UDBAGameInstance::RunAutoPartyStep()
{
	if (bAutoLobbyPartyStepDone || !FParse::Param(FCommandLine::Get(), TEXT("DBAAutoPartyLeader")))
	{
		return;
	}

	UDBAPartyServiceBase* PartyService = GetSubsystem<UDBAPartyServiceBase>();
	UDBAAccountServiceBase* AccountService = GetSubsystem<UDBAAccountServiceBase>();
	if (!PartyService || !AccountService || !AccountService->IsLoggedIn())
	{
		return;
	}

	bAutoLobbyPartyStepDone = true;
	PartyService->CreateParty(FDBAOnPartyCreated::CreateWeakLambda(this, [this, PartyService](const FDBAPartyInfo& PartyInfo)
	{
		if (!PartyInfo.IsValid())
		{
			UE_LOG(LogDBACore, Error, TEXT("[DBAGameInstance] 自动大厅流程：创建队伍失败。"));
			return;
		}

		UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 自动大厅流程：队伍创建成功 %s, 成员=%d。"),
			*PartyInfo.PartyId.ToString(), PartyInfo.Members.Num());

		FString InviteAccountId;
		if (!FParse::Value(FCommandLine::Get(), TEXT("DBAAutoInviteAccountId="), InviteAccountId))
		{
			return;
		}
		InviteAccountId.TrimStartAndEndInline();
		if (InviteAccountId.IsEmpty())
		{
			return;
		}

		PartyService->InvitePlayer(FDBAAccountId(InviteAccountId), FDBAOnPartyOperationComplete::CreateWeakLambda(this, [PartyService, InviteAccountId](bool bSuccess, const FString& ErrorMessage)
		{
			const FDBAPartyInfo& UpdatedParty = PartyService->GetCurrentPartyInfo();
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameInstance] 自动大厅流程：邀请 %s success=%s error=%s members=%d。"),
				*InviteAccountId,
				bSuccess ? TEXT("true") : TEXT("false"),
				*ErrorMessage,
				UpdatedParty.Members.Num());
		}));
	}));
}
