// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Frontend/DBAFrontendFlowController.h"

#include "GameDBA/Core/DBALogChannels.h"

void UDBAFrontendFlowController::Initialize(UDBAFrontendFlowSubsystem* InLoginFlow)
{
	Deinitialize();
	LoginFlow = InLoginFlow;
	if (!LoginFlow.IsValid())
	{
		UE_LOG(LogDBACore, Warning, TEXT("[FrontendFlowController] 登录流程子系统不可用，无法初始化前台流程。"));
		return;
	}

	CurrentState = LoginFlow->GetFlowState();
	LoginFlow->OnFlowStateChanged.AddDynamic(this, &UDBAFrontendFlowController::HandleLoginFlowStateChanged);
}

void UDBAFrontendFlowController::Deinitialize()
{
	if (LoginFlow.IsValid())
	{
		LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBAFrontendFlowController::HandleLoginFlowStateChanged);
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

void UDBAFrontendFlowController::HandleLoginFlowStateChanged(EDBALoginFlowState NewState)
{
	CurrentState = NewState;
	OnViewStateChanged.Broadcast(NewState);
}
