// Copyright FreeboozStudio. All Rights Reserved.

#include "GameCore/Queue/DBAQueueServiceBase.h"

#include "GameCore/Account/DBAAccountServiceBase.h"
#include "GameCore/Party/DBAPartyServiceBase.h"
#include "GameCore/Session/DBAFrontendSessionSubsystem.h"
#include "Misc/DateTime.h"
#include "Misc/Guid.h"

UDBAQueueServiceBase::UDBAQueueServiceBase()
{
}

void UDBAQueueServiceBase::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();
	LogSubsystemInfo(TEXT("Queue service initialized"));
	bIsInitialized = true;
}

void UDBAQueueServiceBase::OnSubsystemDeinitialize()
{
	LogSubsystemInfo(TEXT("Queue service deinitialized"));
	Super::OnSubsystemDeinitialize();
}

bool UDBAQueueServiceBase::IsSupportedInCurrentEnvironment() const
{
	if (GetGameInstance() && GetGameInstance()->IsDedicatedServerInstance())
	{
		return false;
	}

	return true;
}

void UDBAQueueServiceBase::StartQueue(EDBAQueueType QueueType, FDBAOnQueueStarted OnComplete)
{
	if (!EnsureGameThread(TEXT("StartQueue")))
	{
		return;
	}

	UDBAAccountServiceBase* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAAccountServiceBase>() : nullptr;
	UDBAPartyServiceBase* PartyService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAPartyServiceBase>() : nullptr;
	if (!AccountService || !AccountService->IsLoggedIn() || !PartyService || !PartyService->IsInParty())
	{
		LogSubsystemError(TEXT("StartQueue failed: missing logged-in account or party"));
		FDBAQueueInfo EmptyQueue;
		OnComplete.ExecuteIfBound(EmptyQueue);
		return;
	}

	CurrentQueueInfo = FDBAQueueInfo();
	CurrentQueueInfo.QueueId = GenerateQueueId();
	CurrentQueueInfo.QueueType = QueueType;
	CurrentQueueInfo.PartyId = PartyService->GetCurrentPartyInfo().PartyId;
	CurrentQueueInfo.State = EDBAQueueState::Searching;
	CurrentQueueInfo.StartTime = FDateTime::UtcNow().ToUnixTimestamp();
	CurrentQueueInfo.EstimatedWaitTime = 3;

	FDBAQueueUpdate Update;
	Update.QueueInfo = CurrentQueueInfo;
	Update.CurrentQueueSize = 1;
	Update.UpdateTime = FDateTime::UtcNow().ToUnixTimestamp();
	OnQueueUpdated.ExecuteIfBound(Update);

	FDBAMatchFoundNotification MatchFound;
	MatchFound.NotificationTime = FDateTime::UtcNow().ToUnixTimestamp();
	MatchFound.ReadyCheckTimeoutSeconds = 30;
	MatchFound.SessionInfo.SessionId = GenerateMatchSessionId();
	MatchFound.SessionInfo.State = EDBAMatchSessionState::ReadyCheck;
	MatchFound.SessionInfo.MapName = TEXT("Arena_5v5");
	MatchFound.SessionInfo.GameMode = TEXT("Arena");
	MatchFound.SessionInfo.ServerAddress = TEXT("127.0.0.1");
	MatchFound.SessionInfo.ServerPort = 7777;
	MatchFound.SessionInfo.CreateTime = MatchFound.NotificationTime;

	FDBAPlayerMatchInfo SelfPlayer;
	const FDBAAccountInfo& AccountInfo = AccountService->GetCurrentAccountInfo();
	SelfPlayer.AccountId = AccountInfo.AccountId;
	SelfPlayer.DisplayName = AccountInfo.DisplayName;
	SelfPlayer.TeamId = 0;
	SelfPlayer.bIsReady = false;
	MatchFound.SessionInfo.Players.Add(SelfPlayer);

	CurrentReadyCheckInfo = FDBAReadyCheckInfo();
	CurrentReadyCheckInfo.ReadyCheckId = GenerateReadyCheckId();
	CurrentReadyCheckInfo.MatchSessionId = MatchFound.SessionInfo.SessionId;
	CurrentReadyCheckInfo.StartTime = FDateTime::UtcNow().ToUnixTimestamp();
	CurrentReadyCheckInfo.TimeoutSeconds = MatchFound.ReadyCheckTimeoutSeconds;
	CurrentReadyCheckInfo.OverallState = EDBAReadyCheckState::Pending;

	FDBAPlayerReadyStatus SelfReady;
	SelfReady.AccountId = AccountInfo.AccountId;
	SelfReady.DisplayName = AccountInfo.DisplayName;
	SelfReady.State = EDBAReadyCheckState::Pending;
	CurrentReadyCheckInfo.PlayerStatuses.Add(SelfReady);

	if (UDBAFrontendSessionSubsystem* FrontendSession = GetGameInstance()->GetSubsystem<UDBAFrontendSessionSubsystem>())
	{
		FrontendSession->SetCurrentQueueInfo(CurrentQueueInfo);
		FrontendSession->SetCurrentReadyCheckInfo(CurrentReadyCheckInfo);
		FrontendSession->SetCurrentMatchSessionInfo(MatchFound.SessionInfo);
		FrontendSession->SetState(EDBAFrontendSessionState::InQueue);
	}

	OnMatchFound.ExecuteIfBound(MatchFound);
	OnReadyCheckStarted.ExecuteIfBound(CurrentReadyCheckInfo);
	OnComplete.ExecuteIfBound(CurrentQueueInfo);
}

void UDBAQueueServiceBase::CancelQueue(FDBAOnQueueCancelled OnComplete)
{
	if (!EnsureGameThread(TEXT("CancelQueue")))
	{
		return;
	}

	CurrentQueueInfo = FDBAQueueInfo();
	CurrentReadyCheckInfo = FDBAReadyCheckInfo();

	if (UDBAFrontendSessionSubsystem* FrontendSession = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendSessionSubsystem>() : nullptr)
	{
		FrontendSession->ClearCurrentQueueInfo();
		FrontendSession->ClearCurrentReadyCheckInfo();
		FrontendSession->SetState(EDBAFrontendSessionState::InParty);
	}

	LogSubsystemInfo(TEXT("CancelQueue succeeded"));
	OnComplete.ExecuteIfBound();
}

void UDBAQueueServiceBase::ConfirmReady(const FDBAReadyCheckId& ReadyCheckId, FDBAOnReadyCheckCompleted OnComplete)
{
	if (!EnsureGameThread(TEXT("ConfirmReady")))
	{
		return;
	}

	if (!CurrentReadyCheckInfo.IsValid() || CurrentReadyCheckInfo.ReadyCheckId != ReadyCheckId)
	{
		OnComplete.ExecuteIfBound(false);
		return;
	}

	UDBAAccountServiceBase* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAAccountServiceBase>() : nullptr;
	if (!AccountService || !AccountService->IsLoggedIn())
	{
		OnComplete.ExecuteIfBound(false);
		return;
	}

	FDBAPlayerReadyStatus* PlayerReady = CurrentReadyCheckInfo.FindPlayerStatus(AccountService->GetCurrentAccountInfo().AccountId);
	if (!PlayerReady)
	{
		OnComplete.ExecuteIfBound(false);
		return;
	}

	PlayerReady->State = EDBAReadyCheckState::Confirmed;
	PlayerReady->ResponseTime = FDateTime::UtcNow().ToUnixTimestamp();
	CurrentReadyCheckInfo.OverallState = EDBAReadyCheckState::Completed;
	OnReadyCheckUpdated.ExecuteIfBound(CurrentReadyCheckInfo);

	if (UDBAFrontendSessionSubsystem* FrontendSession = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendSessionSubsystem>() : nullptr)
	{
		FrontendSession->SetCurrentReadyCheckInfo(CurrentReadyCheckInfo);

		FDBAMatchSessionInfo MatchInfo = FrontendSession->GetCurrentMatchSessionInfo();
		MatchInfo.State = EDBAMatchSessionState::Loading;
		FrontendSession->SetCurrentMatchSessionInfo(MatchInfo);
		FrontendSession->SetState(EDBAFrontendSessionState::Loading);
	}

	OnReadyCheckCompleted.ExecuteIfBound(true);
	OnComplete.ExecuteIfBound(true);
}

void UDBAQueueServiceBase::DeclineReady(const FDBAReadyCheckId& ReadyCheckId, FDBAOnReadyCheckCompleted OnComplete)
{
	if (!EnsureGameThread(TEXT("DeclineReady")))
	{
		return;
	}

	if (!CurrentReadyCheckInfo.IsValid() || CurrentReadyCheckInfo.ReadyCheckId != ReadyCheckId)
	{
		OnComplete.ExecuteIfBound(false);
		return;
	}

	CurrentReadyCheckInfo.OverallState = EDBAReadyCheckState::Declined;
	OnReadyCheckUpdated.ExecuteIfBound(CurrentReadyCheckInfo);
	OnReadyCheckCompleted.ExecuteIfBound(false);
	OnComplete.ExecuteIfBound(false);
}

FDBAQueueId UDBAQueueServiceBase::GenerateQueueId()
{
	return FDBAQueueId(FGuid::NewGuid().ToString());
}

FDBAReadyCheckId UDBAQueueServiceBase::GenerateReadyCheckId()
{
	return FDBAReadyCheckId(FGuid::NewGuid().ToString());
}

FDBAMatchSessionId UDBAQueueServiceBase::GenerateMatchSessionId()
{
	return FDBAMatchSessionId(FGuid::NewGuid().ToString());
}

