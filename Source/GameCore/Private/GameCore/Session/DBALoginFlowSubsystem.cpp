// Copyright FreeboozStudio. All Rights Reserved.

#include "GameCore/Session/DBALoginFlowSubsystem.h"

#include "GameCore/Account/DBAOnlineAccountService.h"
#include "GameCore/Session/DBAFrontendSessionSubsystem.h"
#include "GameCore/Core/DBALogChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "GameFramework/PlayerController.h"

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

	UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] FlowState: %d -> %d"), static_cast<int32>(FlowState), static_cast<int32>(NewState));
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
	UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] SubmitGuestLogin"));
	SetFlowState(EDBALoginFlowState::TryAutoLogin);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("Account service unavailable"), EDBALoginFlowState::LoginScreen);
		return;
	}

	AccountService->GuestLogin(FDBAOnLoginComplete::CreateWeakLambda(this, [this](const FDBALoginResponse& Response)
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] GuestLogin response: success=%s, error=%s"),
			Response.bSuccess ? TEXT("true") : TEXT("false"), *Response.ErrorMessage);
		if (Response.bSuccess)
		{
			LoadCharactersAfterLogin();
			return;
		}

		BroadcastErrorAndSetState(Response.ErrorMessage, EDBALoginFlowState::LoginScreen);
	}));
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
		UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] CharacterList loaded: count=%d"), CachedCharacters.Num());
		SetFlowState(ShouldEnterCharacterCreate(CachedCharacters.Num()) ? EDBALoginFlowState::CharacterCreate : EDBALoginFlowState::CharacterSelect);
	}));
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
				UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] EnterMainLobby -> ClientTravel to shared lobby server: %s"), *LobbyServerAddress);
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
			UE_LOG(LogDBACore, Log, TEXT("[DBALoginFlowSubsystem] EnterMainLobby -> OpenLevel: %s"), *MainLobbyMapPath);
			UGameplayStatics::OpenLevel(World, FName(*MainLobbyMapPath));
		}
	}
}
