// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameCore/Session/DBALoginFlowSubsystem.h"

#include "GameCore/Account/DBAOnlineAccountService.h"
#include "GameCore/Session/DBAFrontendSessionSubsystem.h"
#include "GameCore/Core/DBALogChannels.h"
#include "GameBackendAuthService.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendConfigService.h"
#include "GameBackendHttpClient.h"
#include "GameBackendPlayerService.h"
#include "GameBackendTelemetryService.h"
#include "Dom/JsonObject.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "GameFramework/PlayerController.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
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

	EDBAZodiac ResolveSelectedLobbyZodiac(const TArray<FDBACharacterSummary>& Characters, const FDBACharacterId& SelectedCharacterId)
	{
		const FDBACharacterSummary* SelectedCharacter = Characters.FindByPredicate(
			[&SelectedCharacterId](const FDBACharacterSummary& Character)
			{
				return Character.CharacterId == SelectedCharacterId;
			});

		if (!SelectedCharacter)
		{
			return EDBAZodiac::None;
		}

		const EDBAZodiac NameZodiac = ParseZodiacName(SelectedCharacter->CharacterName);
		if (NameZodiac != EDBAZodiac::None)
		{
			return NameZodiac;
		}

		return SelectedCharacter->Zodiac == EDBAZodiac::None
			? SelectedCharacter->DefaultZodiac
			: SelectedCharacter->Zodiac;
	}

	void AppendLobbyTravelOptions(FString& LobbyServerAddress, EDBAZodiac Zodiac)
	{
		if (Zodiac == EDBAZodiac::None)
		{
			return;
		}

		LobbyServerAddress += FString::Printf(TEXT("?DBALobbyZodiac=%d"), static_cast<int32>(Zodiac));
	}

	bool TryExtractAuthTokens(const FString& DataJson, FString& OutAccessToken, FString& OutRefreshToken, FString& OutPlayerId)
	{
		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DataJson);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			return false;
		}

		const auto ReadByKeys = [&Root](const TArray<FString>& Keys) -> FString
		{
			for (const FString& Key : Keys)
			{
				FString Value;
				if (Root->TryGetStringField(Key, Value))
				{
					return Value;
				}
			}
			return FString();
		};

		OutAccessToken = ReadByKeys({ TEXT("accessToken"), TEXT("access_token"), TEXT("token") });
		OutRefreshToken = ReadByKeys({ TEXT("refreshToken"), TEXT("refresh_token") });
		OutPlayerId = ReadByKeys({ TEXT("playerId"), TEXT("player_id"), TEXT("uid") });

		if (OutAccessToken.IsEmpty() && Root->HasTypedField<EJson::Object>(TEXT("tokens")))
		{
			const TSharedPtr<FJsonObject> TokensObj = Root->GetObjectField(TEXT("tokens"));
			if (TokensObj.IsValid())
			{
				TokensObj->TryGetStringField(TEXT("accessToken"), OutAccessToken);
				TokensObj->TryGetStringField(TEXT("refreshToken"), OutRefreshToken);
			}
		}
		if (OutPlayerId.IsEmpty() && Root->HasTypedField<EJson::Object>(TEXT("player")))
		{
			const TSharedPtr<FJsonObject> PlayerObj = Root->GetObjectField(TEXT("player"));
			if (PlayerObj.IsValid())
			{
				PlayerObj->TryGetStringField(TEXT("playerId"), OutPlayerId);
			}
		}

		return !OutAccessToken.IsEmpty();
	}

	FString BuildBanErrorMessage(const FString& Code, const FString& Message, const FString& DataJson)
	{
		if (!Code.Contains(TEXT("BAN"), ESearchCase::IgnoreCase))
		{
			return Message;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DataJson);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			return Message;
		}

		FString Reason;
		Root->TryGetStringField(TEXT("banReason"), Reason);
		if (Reason.IsEmpty())
		{
			Root->TryGetStringField(TEXT("reason"), Reason);
		}
		FString UnbanTime;
		Root->TryGetStringField(TEXT("unbanTimeUtc"), UnbanTime);
		if (UnbanTime.IsEmpty())
		{
			Root->TryGetStringField(TEXT("unbanTime"), UnbanTime);
		}

		FString Out = Message.IsEmpty() ? TEXT("\u8d26\u53f7\u5df2\u88ab\u5c01\u7981\u3002") : Message;
		if (!Reason.IsEmpty())
		{
			Out += FString::Printf(TEXT("\n封禁原因: %s"), *Reason);
		}
		if (!UnbanTime.IsEmpty())
		{
			Out += FString::Printf(TEXT("\n解封时间(UTC): %s"), *UnbanTime);
		}
		return Out;
	}
}

bool UDBALoginFlowSubsystem::ShouldEnterCharacterCreate(int32 CharacterCount)
{
	return CharacterCount <= 0;
}

void UDBALoginFlowSubsystem::SetFlowState(EDBALoginFlowState NewState)
{
	if (FlowState == NewState)
	{
		return;
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] 流程状态切换：%d -> %d"), static_cast<int32>(FlowState), static_cast<int32>(NewState));
	FlowState = NewState;
	OnFlowStateChanged.Broadcast(NewState);
}

void UDBALoginFlowSubsystem::BroadcastErrorAndSetState(const FString& ErrorMessage, EDBALoginFlowState NewState)
{
	OnFlowError.Broadcast(ErrorMessage);
	SetFlowState(NewState);
}

void UDBALoginFlowSubsystem::StartLoginFlow()
{
	// UX rule: entering frontend should always land on login screen first.
	// Actual authentication flow starts only after explicit user action.
	SetFlowState(EDBALoginFlowState::LoginScreen);
}

void UDBALoginFlowSubsystem::SubmitLogin(const FString& Email, const FString& Password)
{
	SetFlowState(EDBALoginFlowState::TryAutoLogin);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::LoginScreen);
		return;
	}

	FDBALoginRequest Request;
	Request.LoginType = EDBALoginType::Email;
	Request.Email = Email;
	Request.Password = Password;

	AccountService->Login(Request, FDBAOnLoginComplete::CreateWeakLambda(this, [this](const FDBALoginResponse& Response)
	{
		if (Response.bSuccess)
		{
			LoadCharactersAfterLogin();
			return;
		}

		BroadcastErrorAndSetState(Response.ErrorMessage, EDBALoginFlowState::LoginScreen);
	}));
}

void UDBALoginFlowSubsystem::SubmitGuestLogin()
{
	SetFlowState(EDBALoginFlowState::TryAutoLogin);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::LoginScreen);
		return;
	}

	AccountService->GuestLogin(FDBAOnLoginComplete::CreateWeakLambda(this, [this](const FDBALoginResponse& Response)
	{
		if (Response.bSuccess)
		{
			LoadCharactersAfterLogin();
			return;
		}

		BroadcastErrorAndSetState(Response.ErrorMessage, EDBALoginFlowState::LoginScreen);
	}));
}

void UDBALoginFlowSubsystem::SubmitDebugLogin(const FString& Username)
{
#if UE_BUILD_SHIPPING
	UE_LOG(LogDBACore, Warning, TEXT("[DBALoginFlowSubsystem] Shipping 构建禁用开发账号登录。"));
	BroadcastErrorAndSetState(TEXT("当前版本不支持开发账号登录。"), EDBALoginFlowState::LoginScreen);
#else
	UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] 发起开发账号登录：%s"), *Username);
	SetFlowState(EDBALoginFlowState::TryAutoLogin);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::LoginScreen);
		return;
	}

	FDBALoginRequest Request;
	Request.LoginType = EDBALoginType::Email;
	Request.Email = Username.IsEmpty() ? TEXT("dba_dev_01") : Username;
	Request.Password = TEXT("Dev@123456");

	AccountService->Login(Request, FDBAOnLoginComplete::CreateWeakLambda(this, [this](const FDBALoginResponse& Response)
	{
		if (Response.bSuccess)
		{
			LoadCharactersAfterLogin();
			return;
		}

		BroadcastErrorAndSetState(Response.ErrorMessage.IsEmpty() ? TEXT("开发账号登录失败。") : Response.ErrorMessage, EDBALoginFlowState::LoginScreen);
	}));
#endif
}

void UDBALoginFlowSubsystem::LoadCharactersAfterLogin()
{
	SetFlowState(EDBALoginFlowState::LoadCharacterList);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::LoginScreen);
		return;
	}

	AccountService->GetCharacterList(FDBAOnCharacterListLoaded::CreateWeakLambda(this, [this](const TArray<FDBACharacterSummary>& Characters)
	{
		CachedCharacters = Characters;
		OnCharactersLoaded.Broadcast(CachedCharacters);
		UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] 角色列表加载完成：%d"), CachedCharacters.Num());
		SetFlowState(ShouldEnterCharacterCreate(CachedCharacters.Num()) ? EDBALoginFlowState::CharacterCreate : EDBALoginFlowState::CharacterSelect);
	}));
}

void UDBALoginFlowSubsystem::FetchPostLoginDataAndEnterLobby()
{
	UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr;
	UDBA_GameBackendPlayerService* PlayerService = Backend ? Backend->GetPlayerService() : nullptr;
	UDBA_GameBackendConfigService* ConfigService = Backend ? Backend->GetConfigService() : nullptr;
	if (!Backend || !PlayerService || !ConfigService)
	{
		BroadcastErrorAndSetState(TEXT("\u540e\u7aef\u670d\u52a1\u4e0d\u53ef\u7528\u3002"), EDBALoginFlowState::LoginScreen);
		return;
	}

	FDBA_GameBackendResponseDelegate ProfileCallback;
	ProfileCallback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBALoginFlowSubsystem, HandleDebugLoginProfileResponse));
	PlayerService->GetMyProfile(ProfileCallback);
}

void UDBALoginFlowSubsystem::HandleDebugLoginAuthResponse(
	bool bSuccess,
	const FString& ErrorMessage,
	const FString& AccessToken,
	const FString& RefreshToken,
	const FString& PlayerId)
{
	UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr;
	if (!Backend)
	{
		BroadcastErrorAndSetState(TEXT("\u540e\u7aef\u5ba2\u6237\u7aef\u672a\u521d\u59cb\u5316\u3002"), EDBALoginFlowState::LoginScreen);
		return;
	}

	if (bSuccess)
	{
		if (AccessToken.IsEmpty())
		{
			BroadcastErrorAndSetState(TEXT("\u767b\u5f55\u6210\u529f\uff0c\u4f46\u672a\u8fd4\u56de\u6709\u6548\u4ee4\u724c\u3002"), EDBALoginFlowState::LoginScreen);
			return;
		}

		Backend->SetAuthTokens(AccessToken, RefreshToken, PlayerId);
		if (Backend->GetTelemetryService())
		{
			TMap<FString, FString> Props;
			Props.Add(TEXT("loginType"), TEXT("dev"));
			Props.Add(TEXT("playerId"), PlayerId);
			Backend->GetTelemetryService()->TrackEvent(TEXT("login_success"), Props);
		}

		FetchPostLoginDataAndEnterLobby();
		return;
	}

	if (Backend->GetTelemetryService())
	{
		TMap<FString, FString> Props;
		Props.Add(TEXT("loginType"), TEXT("dev"));
		Props.Add(TEXT("error"), ErrorMessage.Left(256));
		Backend->GetTelemetryService()->TrackEvent(TEXT("login_failed"), Props);
	}
	BroadcastErrorAndSetState(ErrorMessage.IsEmpty() ? TEXT("\u767b\u5f55\u5931\u8d25\u3002") : ErrorMessage, EDBALoginFlowState::LoginScreen);
}

void UDBALoginFlowSubsystem::HandleDebugLoginProfileResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		BroadcastErrorAndSetState(ErrorMessage.IsEmpty() ? TEXT("\u62c9\u53d6\u73a9\u5bb6\u8d44\u6599\u5931\u8d25\u3002") : ErrorMessage, EDBALoginFlowState::LoginScreen);
		return;
	}

	UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr;
	UDBA_GameBackendConfigService* ConfigService = Backend ? Backend->GetConfigService() : nullptr;
	if (!ConfigService)
	{
		BroadcastErrorAndSetState(TEXT("\u540e\u7aef\u670d\u52a1\u4e0d\u53ef\u7528\u3002"), EDBALoginFlowState::LoginScreen);
		return;
	}

	FDBA_GameBackendResponseDelegate ConfigCallback;
	ConfigCallback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBALoginFlowSubsystem, HandleDebugLoginConfigResponse));
	ConfigService->GetConfigBundle(ConfigCallback);
}

void UDBALoginFlowSubsystem::HandleDebugLoginConfigResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		BroadcastErrorAndSetState(ErrorMessage.IsEmpty() ? TEXT("\u62c9\u53d6\u914d\u7f6e\u5931\u8d25\u3002") : ErrorMessage, EDBALoginFlowState::LoginScreen);
		return;
	}

	EnterMainLobby();
}

void UDBALoginFlowSubsystem::SubmitCharacterSelection(const FDBACharacterId& CharacterId)
{
	if (FlowState != EDBALoginFlowState::CharacterSelect && FlowState != EDBALoginFlowState::CharacterCreate)
	{
		BroadcastErrorAndSetState(TEXT("当前状态不允许选择角色。"), EDBALoginFlowState::LoginScreen);
		return;
	}

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::CharacterSelect);
		return;
	}

	const EDBAZodiac CachedZodiac = ResolveSelectedLobbyZodiac(CachedCharacters, CharacterId);
	if (CachedZodiac != EDBAZodiac::None)
	{
		CurrentSelectedLobbyZodiac = CachedZodiac;
	}

	AccountService->SelectCharacter(CharacterId, FDBAOnCharacterSelected::CreateWeakLambda(this, [this](const FDBACharacterId& SelectedId)
	{
		if (SelectedId.IsValid())
		{
			EnterMainLobby();
			return;
		}

		BroadcastErrorAndSetState(TEXT("角色选择失败。"), EDBALoginFlowState::CharacterSelect);
	}));
}

void UDBALoginFlowSubsystem::SubmitCharacterCreation(const FDBACharacterCreateRequest& Request)
{
	if (FlowState != EDBALoginFlowState::CharacterCreate)
	{
		BroadcastErrorAndSetState(TEXT("当前状态不允许创建角色。"), EDBALoginFlowState::LoginScreen);
		return;
	}

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::CharacterCreate);
		return;
	}

	AccountService->CreateCharacter(Request, FDBAOnCharacterCreated::CreateWeakLambda(this, [this](const FDBACharacterCreateResponse& Response)
	{
		if (Response.bSuccess)
		{
			CurrentSelectedLobbyZodiac = Response.CharacterSummary.Zodiac == EDBAZodiac::None
				? Response.CharacterSummary.DefaultZodiac
				: Response.CharacterSummary.Zodiac;
			SubmitCharacterSelection(Response.CharacterSummary.CharacterId);
			return;
		}

		BroadcastErrorAndSetState(Response.ErrorMessage, EDBALoginFlowState::CharacterCreate);
	}));
}

void UDBALoginFlowSubsystem::EnterCharacterCreate()
{
	if (FlowState == EDBALoginFlowState::CharacterSelect || FlowState == EDBALoginFlowState::CharacterCreate)
	{
		SetFlowState(EDBALoginFlowState::CharacterCreate);
	}
}

void UDBALoginFlowSubsystem::BackToCharacterSelect()
{
	if (FlowState == EDBALoginFlowState::CharacterCreate || FlowState == EDBALoginFlowState::CharacterSelect)
	{
		SetFlowState(EDBALoginFlowState::CharacterSelect);
	}
}

void UDBALoginFlowSubsystem::RefreshCharacterList()
{
	LoadCharactersAfterLogin();
}

void UDBALoginFlowSubsystem::EnterMainLobby()
{
	if (UDBAFrontendSessionSubsystem* FrontendSession = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendSessionSubsystem>() : nullptr)
	{
		FrontendSession->SetState(EDBAFrontendSessionState::MainLobby);
	}

	SetFlowState(EDBALoginFlowState::MainLobby);
	if (UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr)
	{
		if (Backend->GetTelemetryService())
		{
			Backend->GetTelemetryService()->TrackEvent(TEXT("enter_lobby"), TMap<FString, FString>());
		}
	}

	if (UWorld* World = GetWorld())
	{
		FString LobbyServerAddress;
		if (GConfig)
		{
			GConfig->GetString(TEXT("/Script/DivineBeastsArena.DBAFrontendConfig"), TEXT("SharedLobbyServerAddress"), LobbyServerAddress, GGameIni);
			LobbyServerAddress = LobbyServerAddress.TrimStartAndEnd();
		}

		// Preferred path for multiplayer lobby: everyone connects to the same lobby server.
		if (!LobbyServerAddress.IsEmpty())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				EDBAZodiac LobbyZodiac = CurrentSelectedLobbyZodiac;
				if (LobbyZodiac == EDBAZodiac::None)
				{
					if (UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr)
					{
						LobbyZodiac = ResolveSelectedLobbyZodiac(CachedCharacters, AccountService->GetCurrentCharacterId());
					}
				}
				AppendLobbyTravelOptions(LobbyServerAddress, LobbyZodiac);
				UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] 进入大厅，ClientTravel 到共享大厅服务器：%s"), *LobbyServerAddress);
				PC->ClientTravel(LobbyServerAddress, TRAVEL_Absolute);
				return;
			}
		}

		FString MainLobbyMapPath = TEXT("/Game/Maps/Lobby/LobbyMap");
		if (GConfig)
		{
			FString ConfigMapPath;
			if (GConfig->GetString(TEXT("/Script/DivineBeastsArena.DBAFrontendConfig"), TEXT("DefaultLobbyMap"), ConfigMapPath, GGameIni))
			{
				ConfigMapPath = ConfigMapPath.TrimStartAndEnd();
				if (!ConfigMapPath.IsEmpty())
				{
					MainLobbyMapPath = ConfigMapPath;
				}
			}
		}

		FString CurrentLevelPath;
		if (World->PersistentLevel)
		{
			CurrentLevelPath = World->PersistentLevel->GetOutermost()->GetName();
		}

		if (!CurrentLevelPath.Contains(TEXT("LobbyMap")))
		{
			UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] 进入大厅，OpenLevel：%s"), *MainLobbyMapPath);
			UGameplayStatics::OpenLevel(World, FName(*MainLobbyMapPath));
		}
	}
}
