// Copyright FreeboozStudio. All Rights Reserved.

#include "GameCore/Session/DBALoginFlowSubsystem.h"

#include "GameCore/Account/DBAOnlineAccountService.h"
#include "GameCore/Session/DBAFrontendSessionSubsystem.h"

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
	SetFlowState(EDBALoginFlowState::TryAutoLogin);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		SetFlowState(EDBALoginFlowState::LoginScreen);
		return;
	}

	AccountService->AutoLogin(FDBAOnLoginComplete::CreateWeakLambda(this, [this](const FDBALoginResponse& Response)
	{
		if (Response.bSuccess)
		{
			LoadCharactersAfterLogin();
			return;
		}

		SetFlowState(EDBALoginFlowState::LoginScreen);
	}));
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
	SetFlowState(EDBALoginFlowState::TryAutoLogin);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("Account service unavailable"), EDBALoginFlowState::LoginScreen);
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
		SetFlowState(ShouldEnterCharacterCreate(CachedCharacters.Num()) ? EDBALoginFlowState::CharacterCreate : EDBALoginFlowState::CharacterSelect);
	}));
}

void UDBALoginFlowSubsystem::SubmitCharacterSelection(const FDBACharacterId& CharacterId)
{
	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("Account service unavailable"), EDBALoginFlowState::CharacterSelect);
		return;
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
			SubmitCharacterSelection(Response.CharacterSummary.CharacterId);
			return;
		}

		BroadcastErrorAndSetState(Response.ErrorMessage, EDBALoginFlowState::CharacterCreate);
	}));
}

void UDBALoginFlowSubsystem::EnterMainLobby()
{
	if (UDBAFrontendSessionSubsystem* FrontendSession = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendSessionSubsystem>() : nullptr)
	{
		FrontendSession->SetState(EDBAFrontendSessionState::MainLobby);
	}

	SetFlowState(EDBALoginFlowState::MainLobby);
}
