// Copyright FreeboozStudio. All Rights Reserved.

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
			Out += FString::Printf(TEXT("\n灏佺鍘熷洜锛?s"), *Reason);
		}
		if (!UnbanTime.IsEmpty())
		{
			Out += FString::Printf(TEXT("\n瑙ｅ皝鏃堕棿(UTC)锛?s"), *UnbanTime);
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

	UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] 娴佺▼鐘舵€佸垏鎹? %d -> %d"), static_cast<int32>(FlowState), static_cast<int32>(NewState));
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
		BroadcastErrorAndSetState(TEXT("Account service unavailable"), EDBALoginFlowState::LoginScreen);
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
	SubmitDebugLogin(TEXT("frontend_debug"));
}

void UDBALoginFlowSubsystem::SubmitDebugLogin(const FString& DisplayName)
{
	UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] 鍙戣捣璋冭瘯鐧诲綍锛岃处鍙?%s"), *DisplayName);
	SetFlowState(EDBALoginFlowState::TryAutoLogin);

	UGameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGameBackendClientSubsystem>() : nullptr;
	if (!Backend || !Backend->GetHttpClient())
	{
		BroadcastErrorAndSetState(TEXT("\u540e\u7aef\u5ba2\u6237\u7aef\u672a\u521d\u59cb\u5316\u3002"), EDBALoginFlowState::LoginScreen);
		return;
	}

	const TSharedRef<FJsonObject> Request = MakeShared<FJsonObject>();
	Request->SetStringField(TEXT("displayName"), DisplayName);
	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Request, Writer);

	Backend->GetHttpClient()->Post(TEXT("/api/auth/dev-login"), Body, [this, Backend](const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		if (bSuccess)
		{
			FString AccessToken;
			FString RefreshToken;
			FString PlayerId;
			if (!TryExtractAuthTokens(Result.DataJson, AccessToken, RefreshToken, PlayerId))
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

		const FString ErrorMessage = BuildBanErrorMessage(Result.Code, Result.Message, Result.DataJson);
		if (Backend->GetTelemetryService())
		{
			TMap<FString, FString> Props;
			Props.Add(TEXT("loginType"), TEXT("dev"));
			Props.Add(TEXT("error"), ErrorMessage.Left(256));
			Backend->GetTelemetryService()->TrackEvent(TEXT("login_failed"), Props);
		}
		BroadcastErrorAndSetState(ErrorMessage.IsEmpty() ? TEXT("\u767b\u5f55\u5931\u8d25\u3002") : ErrorMessage, EDBALoginFlowState::LoginScreen);
	}, false);
}

void UDBALoginFlowSubsystem::LoadCharactersAfterLogin()
{
	SetFlowState(EDBALoginFlowState::LoadCharacterList);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("Account service unavailable"), EDBALoginFlowState::LoginScreen);
		return;
	}

	AccountService->GetCharacterList(FDBAOnCharacterListLoaded::CreateWeakLambda(this, [this](const TArray<FDBACharacterSummary>& Characters)
	{
		CachedCharacters = Characters;
		OnCharactersLoaded.Broadcast(CachedCharacters);
		UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] 瑙掕壊鍒楄〃鍔犺浇瀹屾垚锛屾暟閲?%d"), CachedCharacters.Num());
		SetFlowState(ShouldEnterCharacterCreate(CachedCharacters.Num()) ? EDBALoginFlowState::CharacterCreate : EDBALoginFlowState::CharacterSelect);
	}));
}

void UDBALoginFlowSubsystem::FetchPostLoginDataAndEnterLobby()
{
	UGameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGameBackendClientSubsystem>() : nullptr;
	if (!Backend || !Backend->GetHttpClient())
	{
		BroadcastErrorAndSetState(TEXT("\u540e\u7aef\u670d\u52a1\u4e0d\u53ef\u7528\u3002"), EDBALoginFlowState::LoginScreen);
		return;
	}

	Backend->GetHttpClient()->Get(TEXT("/api/players/me/profile"), [this, Backend](const FGameBackendHttpResult& ProfileResult)
	{
		const bool bProfileOk = ProfileResult.bHttpRequestOk && ProfileResult.HttpStatus >= 200 && ProfileResult.HttpStatus < 300;
		if (!bProfileOk)
		{
			BroadcastErrorAndSetState(ProfileResult.Message.IsEmpty() ? TEXT("\u62c9\u53d6\u73a9\u5bb6\u8d44\u6599\u5931\u8d25\u3002") : ProfileResult.Message, EDBALoginFlowState::LoginScreen);
			return;
		}

		Backend->GetHttpClient()->Get(TEXT("/api/config/bundle"), [this](const FGameBackendHttpResult& ConfigResult)
		{
			const bool bConfigOk = ConfigResult.bHttpRequestOk && ConfigResult.HttpStatus >= 200 && ConfigResult.HttpStatus < 300;
			if (!bConfigOk)
			{
				BroadcastErrorAndSetState(ConfigResult.Message.IsEmpty() ? TEXT("\u62c9\u53d6\u914d\u7f6e\u5931\u8d25\u3002") : ConfigResult.Message, EDBALoginFlowState::LoginScreen);
				return;
			}

			EnterMainLobby();
		});
	});
}

void UDBALoginFlowSubsystem::SubmitCharacterSelection(const FDBACharacterId& CharacterId)
{
	if (FlowState != EDBALoginFlowState::CharacterSelect && FlowState != EDBALoginFlowState::CharacterCreate)
	{
		BroadcastErrorAndSetState(TEXT("Character selection is not available in current state"), EDBALoginFlowState::LoginScreen);
		return;
	}

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("Account service unavailable"), EDBALoginFlowState::CharacterSelect);
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

		BroadcastErrorAndSetState(TEXT("Character selection failed"), EDBALoginFlowState::CharacterSelect);
	}));
}

void UDBALoginFlowSubsystem::SubmitCharacterCreation(const FDBACharacterCreateRequest& Request)
{
	if (FlowState != EDBALoginFlowState::CharacterCreate)
	{
		BroadcastErrorAndSetState(TEXT("Character creation is not available in current state"), EDBALoginFlowState::LoginScreen);
		return;
	}

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("Account service unavailable"), EDBALoginFlowState::CharacterCreate);
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
	if (UGameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGameBackendClientSubsystem>() : nullptr)
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
				UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] 杩涘叆澶у巺锛欳lientTravel 鍒板叡浜ぇ鍘呮湇 %s"), *LobbyServerAddress);
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
			UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] 杩涘叆澶у巺锛歄penLevel %s"), *MainLobbyMapPath);
			UGameplayStatics::OpenLevel(World, FName(*MainLobbyMapPath));
		}
	}
}
