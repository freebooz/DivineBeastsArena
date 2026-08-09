// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/ServerDirectory/DBAServerSelectWidgetController.h"

#include "GameBackendClientSettings.h"
#include "GameDBA/Frontend/Core/DBAFrontendErrorMapper.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameDBA/Frontend/ServerDirectory/DBAServerDirectorySubsystem.h"
#include "GameDBA/Frontend/ServerDirectory/DBAServerSelectViewModel.h"
#include "Engine/GameInstance.h"

UDBAServerSelectWidgetController::UDBAServerSelectWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAServerSelectWidgetController::Start()
{
	if (!ViewModel)
	{
		ViewModel = NewObject<UDBAServerSelectViewModel>(this);
	}

	UDBAServerDirectorySubsystem* Directory = GetServerDirectory();
	UDBAFrontendFlowSubsystem* Flow = GetFlow();
	if (!Directory || !Flow)
	{
		const FDBAApiError Error = UDBAFrontendErrorMapper::FromLegacyMessage(TEXT("选服服务尚未就绪。"));
		ViewModel->SetLastError(Error);
		OnServerDirectoryError.Broadcast(Error);
		return;
	}

	if (ServerDirectory.Get() != Directory)
	{
		DeinitializeController();
		ServerDirectory = Directory;
		Directory->OnDirectoryChanged.AddDynamic(this, &UDBAServerSelectWidgetController::HandleDirectoryChanged);
		FrontendFlow = Flow;
		Flow->OnFrontendStateChanged.AddDynamic(this, &UDBAServerSelectWidgetController::HandleFlowStateChanged);
	}

	const FString LastServerId = Directory->GetLastSelectedServerId(Flow->GetFrontendSessionContext().AccountId);
	if (!Directory->GetCachedServers().IsEmpty())
	{
		ViewModel->ApplyDirectory(Directory->GetCachedServers(), LastServerId);
	}
	else
	{
		Refresh();
	}
}

void UDBAServerSelectWidgetController::Refresh()
{
	if (!ViewModel || !ViewModel->CanRefresh())
	{
		return;
	}

	UDBAServerDirectorySubsystem* Directory = GetServerDirectory();
	if (!Directory)
	{
		return;
	}

	const UDBAExternalServiceSettings* Settings = GetDefault<UDBAExternalServiceSettings>();
	ViewModel->SetOperationState(EDBAAsyncOperationState::InProgress);
	if (!Directory->RefreshDirectory(
		Settings ? Settings->Region : FString(),
		Settings ? Settings->ClientVersion : FString(),
		Settings ? Settings->Platform : FString())
		&& !Directory->IsRefreshInFlight())
	{
		const FDBAApiError Error = UDBAFrontendErrorMapper::FromLegacyMessage(TEXT("无法发起区服目录刷新。"));
		ViewModel->SetLastError(Error);
		OnServerDirectoryError.Broadcast(Error);
	}
}

void UDBAServerSelectWidgetController::Retry()
{
	Refresh();
}

void UDBAServerSelectWidgetController::SelectServer(const FString& ServerId)
{
	if (ViewModel)
	{
		ViewModel->SelectServer(ServerId);
	}
}

void UDBAServerSelectWidgetController::ConfirmSelection()
{
	if (!ViewModel || !ViewModel->CanConfirmSelection())
	{
		return;
	}

	if (UDBAFrontendFlowSubsystem* Flow = GetFlow())
	{
		ViewModel->SetOperationState(EDBAAsyncOperationState::InProgress);
		Flow->SelectServer(ViewModel->GetSelectedServerId());
	}
}

void UDBAServerSelectWidgetController::RequestBackToLogin()
{
	if (UDBAFrontendFlowSubsystem* Flow = GetFlow())
	{
		Flow->RequestLogout();
	}
}

void UDBAServerSelectWidgetController::HandleDirectoryChanged(const bool bSuccess, const FDBAApiError& Error)
{
	if (!ViewModel)
	{
		return;
	}

	if (!bSuccess)
	{
		ViewModel->SetLastError(Error);
		OnServerDirectoryError.Broadcast(Error);
		return;
	}

	UDBAServerDirectorySubsystem* Directory = GetServerDirectory();
	UDBAFrontendFlowSubsystem* Flow = GetFlow();
	if (Directory && Flow)
	{
		ViewModel->ApplyDirectory(Directory->GetCachedServers(), Directory->GetLastSelectedServerId(Flow->GetFrontendSessionContext().AccountId));
	}
}

void UDBAServerSelectWidgetController::HandleFlowStateChanged(const EDBAFrontendState PreviousState, const EDBAFrontendState NewState)
{
	if (ViewModel && PreviousState == EDBAFrontendState::ServerSelect && NewState == EDBAFrontendState::CharacterRosterLoading)
	{
		ViewModel->SetOperationState(EDBAAsyncOperationState::Succeeded);
	}
}

void UDBAServerSelectWidgetController::DeinitializeController()
{
	if (ServerDirectory.IsValid())
	{
		ServerDirectory->OnDirectoryChanged.RemoveDynamic(this, &UDBAServerSelectWidgetController::HandleDirectoryChanged);
	}
	if (FrontendFlow.IsValid())
	{
		FrontendFlow->OnFrontendStateChanged.RemoveDynamic(this, &UDBAServerSelectWidgetController::HandleFlowStateChanged);
	}
	ServerDirectory.Reset();
	FrontendFlow.Reset();
}

UDBAServerDirectorySubsystem* UDBAServerSelectWidgetController::GetServerDirectory() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBAServerDirectorySubsystem>()
		: nullptr;
}

UDBAFrontendFlowSubsystem* UDBAServerSelectWidgetController::GetFlow() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>()
		: nullptr;
}
