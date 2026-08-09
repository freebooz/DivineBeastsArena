// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Frontend/DBAFrontendFlowController.h"

#include "GameDBA/Frontend/Account/DBAOnlineAccountService.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Engine/GameInstance.h"

void UDBAFrontendFlowController::Initialize(UDBAFrontendFlowSubsystem* InLoginFlow)
{
	Deinitialize();
	LoginFlow = InLoginFlow;
	if (!LoginFlow.IsValid())
	{
		UE_LOG(LogDBACore, Warning, TEXT("[FrontendFlowController] 登录流程子系统不可用，无法初始化前台流程。"));
		return;
	}

	CurrentState = ToLegacyViewState(LoginFlow->GetFrontendState());
	LoginFlow->OnFrontendStateChanged.AddDynamic(this, &UDBAFrontendFlowController::HandleFrontendStateChanged);
}

void UDBAFrontendFlowController::Deinitialize()
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->OnFrontendStateChanged.RemoveDynamic(this, &UDBAFrontendFlowController::HandleFrontendStateChanged);
	}
	LoginFlow.Reset();
}

EDBALoginFlowState UDBAFrontendFlowController::GetCurrentState() const
{
	return CurrentState;
}

const TArray<FDBACharacterSummary>& UDBAFrontendFlowController::GetCachedCharacters() const
{
	static const TArray<FDBACharacterSummary> EmptyCharacters;
	return LoginFlow.IsValid() ? LoginFlow->GetCachedCharacters() : EmptyCharacters;
}

void UDBAFrontendFlowController::StartLoginFlow()
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->StartLoginFlow();
	}
}

void UDBAFrontendFlowController::SubmitLogin(const FString& Email, const FString& Password)
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->SubmitLogin(Email, Password);
	}
}

void UDBAFrontendFlowController::SubmitGuestLogin()
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->SubmitGuestLogin();
	}
}

void UDBAFrontendFlowController::BeginRegistration()
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->BeginRegistration();
	}
}

void UDBAFrontendFlowController::SubmitRegistration(const FString& Account, const FString& Password)
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->SubmitRegistration(Account, Password);
	}
}

void UDBAFrontendFlowController::CancelRegistration()
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->CancelRegistration();
	}
}

void UDBAFrontendFlowController::SetRememberSession(const bool bRemember)
{
	if (LoginFlow.IsValid())
	{
		if (UGameInstance* GameInstance = LoginFlow->GetGameInstance())
		{
			if (UDBAOnlineAccountService* AccountService = GameInstance->GetSubsystem<UDBAOnlineAccountService>())
			{
				AccountService->SetRememberSession(bRemember);
			}
		}
	}
}

void UDBAFrontendFlowController::SubmitCharacterSelection(const FDBACharacterId& CharacterId)
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->SubmitCharacterSelection(CharacterId);
	}
}

void UDBAFrontendFlowController::SubmitCharacterCreation(const FDBACharacterCreateRequest& Request)
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->SubmitCharacterCreation(Request);
	}
}

void UDBAFrontendFlowController::EnterCharacterCreate()
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->EnterCharacterCreate();
	}
}

void UDBAFrontendFlowController::BackToCharacterSelect()
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->BackToCharacterSelect();
	}
}

void UDBAFrontendFlowController::RefreshCharacterList()
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->RefreshCharacterList();
	}
}

void UDBAFrontendFlowController::HandleFrontendStateChanged(EDBAFrontendState PreviousState, EDBAFrontendState NewState)
{
	CurrentState = ToLegacyViewState(NewState);
	OnViewStateChanged.Broadcast(CurrentState);
}

EDBALoginFlowState UDBAFrontendFlowController::ToLegacyViewState(const EDBAFrontendState State)
{
	switch (State)
	{
	case EDBAFrontendState::Bootstrapping:
	case EDBAFrontendState::Startup:
		return EDBALoginFlowState::Booting;
	case EDBAFrontendState::CharacterRosterLoading:
		return EDBALoginFlowState::LoadingCharacters;
	case EDBAFrontendState::ServerSelect:
		return EDBALoginFlowState::ServerSelecting;
	case EDBAFrontendState::CharacterSelect:
		return EDBALoginFlowState::CharacterSelecting;
	case EDBAFrontendState::CharacterCreate_Zodiac:
	case EDBAFrontendState::CharacterCreate_Element:
	case EDBAFrontendState::CharacterCreate_FiveCamp:
	case EDBAFrontendState::CharacterCreate_Confirm:
		return EDBALoginFlowState::CharacterCreating;
	case EDBAFrontendState::EnteringWorld:
		return EDBALoginFlowState::ConnectingVillage;
	case EDBAFrontendState::RecoverableError:
		return EDBALoginFlowState::RecoverableError;
	case EDBAFrontendState::FatalError:
		return EDBALoginFlowState::FatalError;
	default:
		return EDBALoginFlowState::AwaitingLogin;
	}
}
