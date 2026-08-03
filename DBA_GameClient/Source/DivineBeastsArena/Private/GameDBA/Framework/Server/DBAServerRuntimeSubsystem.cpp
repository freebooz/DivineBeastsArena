#include "GameDBA/Framework/Server/DBAServerRuntimeSubsystem.h"

#include "GameBackendClientSubsystem.h"
#include "GameBackendRuntimeService.h"

UDBA_GameBackendRuntimeService* UDBAServerRuntimeSubsystem::ResolveRuntimeService() const
{
	UDBA_GameBackendClientSubsystem* Backend = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>()
		: nullptr;
	return Backend ? Backend->GetRuntimeService() : nullptr;
}

bool UDBAServerRuntimeSubsystem::ConfigureAndRegister()
{
	UDBA_GameBackendRuntimeService* RuntimeService = ResolveRuntimeService();
	if (!RuntimeService || !RuntimeService->ConfigureFromCommandLine())
	{
		return false;
	}

	FDBA_GameBackendResponseDelegate EmptyCallback;
	RuntimeService->RegisterServer(EmptyCallback);
	RuntimeService->MarkReady(EmptyCallback);
	return true;
}

bool UDBAServerRuntimeSubsystem::IsConfigured() const
{
	const UDBA_GameBackendRuntimeService* RuntimeService = ResolveRuntimeService();
	return RuntimeService && RuntimeService->IsConfigured();
}

void UDBAServerRuntimeSubsystem::ValidateJoinTicket(
	const FString& PlayerId,
	const FString& CharacterId,
	const FString& JoinTicket,
	const FString& Team,
	int32 SlotIndex,
	const FDBA_GameBackendRuntimePlayerBuildSummary& BuildSummary,
	FDBA_GameBackendNativeResponseCallback Callback)
{
	if (UDBA_GameBackendRuntimeService* RuntimeService = ResolveRuntimeService())
	{
		RuntimeService->ValidateJoinTicket(PlayerId, CharacterId, JoinTicket, Team, SlotIndex, BuildSummary, MoveTemp(Callback));
		return;
	}
	if (Callback)
	{
		Callback(false, TEXT("后台运行时服务不可用。"), TEXT("{}"));
	}
}

void UDBAServerRuntimeSubsystem::SendHeartbeat()
{
	if (UDBA_GameBackendRuntimeService* RuntimeService = ResolveRuntimeService())
	{
		FDBA_GameBackendResponseDelegate EmptyCallback;
		RuntimeService->SendHeartbeat(EmptyCallback);
	}
}

void UDBAServerRuntimeSubsystem::NotifyPlayerLeft(const FString& PlayerId)
{
	if (UDBA_GameBackendRuntimeService* RuntimeService = ResolveRuntimeService())
	{
		FDBA_GameBackendResponseDelegate EmptyCallback;
		RuntimeService->NotifyPlayerLeft(PlayerId, EmptyCallback);
	}
}

void UDBAServerRuntimeSubsystem::NotifyMatchStarted()
{
	if (UDBA_GameBackendRuntimeService* RuntimeService = ResolveRuntimeService())
	{
		FDBA_GameBackendResponseDelegate EmptyCallback;
		RuntimeService->NotifyMatchStarted(EmptyCallback);
	}
}

void UDBAServerRuntimeSubsystem::NotifyMatchEnded()
{
	if (UDBA_GameBackendRuntimeService* RuntimeService = ResolveRuntimeService())
	{
		FDBA_GameBackendResponseDelegate EmptyCallback;
		RuntimeService->NotifyMatchEnded(EmptyCallback);
	}
}

void UDBAServerRuntimeSubsystem::NotifyMatchResults(
	const FString& IdempotencyKey,
	const FString& ResultJson,
	const TArray<FDBA_GameBackendRuntimePlayerResult>& Players)
{
	if (UDBA_GameBackendRuntimeService* RuntimeService = ResolveRuntimeService())
	{
		FDBA_GameBackendResponseDelegate EmptyCallback;
		RuntimeService->NotifyMatchResults(IdempotencyKey, ResultJson, Players, EmptyCallback);
	}
}
