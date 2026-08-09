#include "GameDBA/Frontend/Backend/DBABackendFacadeSubsystem.h"

#include "GameBackendClientSubsystem.h"
#include "GameBackendSessionService.h"
#include "GameBackendTelemetryService.h"
#include "GameCore/Networking/Account/DBAAccountServiceBase.h"
#include "GameDBA/Frontend/Online/DBAApiClientSubsystem.h"

bool UDBABackendFacadeSubsystem::SynchronizeAuthentication(
	const UDBAAccountServiceBase* AccountService,
	FString& OutErrorMessage)
{
	OutErrorMessage.Reset();
	UDBAApiClientSubsystem* ApiClient = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>()
		: nullptr;
	if (!ApiClient || !AccountService || !AccountService->IsLoggedIn() || AccountService->GetAccessToken().IsEmpty())
	{
		OutErrorMessage = TEXT("后端认证上下文不完整。");
		return false;
	}

	ApiClient->SetAuthenticationTokens(
		AccountService->GetAccessToken(),
		AccountService->GetRefreshToken(),
		AccountService->GetCurrentAccountInfo().AccountId.ToString());
	return true;
}

void UDBABackendFacadeSubsystem::AllocateVillage(
	const FString& CharacterId,
	const FDBA_GameBackendResponseDelegate& Callback)
{
	UDBA_GameBackendClientSubsystem* Backend = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>()
		: nullptr;
	UDBA_GameBackendSessionService* SessionService = Backend ? Backend->GetSessionService() : nullptr;
	if (!SessionService)
	{
		Callback.ExecuteIfBound(false, TEXT("新手村分配服务不可用。"), TEXT("{}"));
		return;
	}
	SessionService->AllocateVillage(CharacterId, Callback);
}

void UDBABackendFacadeSubsystem::GetVillageConnection(
	const FString& SessionId,
	const FDBA_GameBackendResponseDelegate& Callback)
{
	UDBA_GameBackendClientSubsystem* Backend = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>()
		: nullptr;
	UDBA_GameBackendSessionService* SessionService = Backend ? Backend->GetSessionService() : nullptr;
	if (!SessionService)
	{
		Callback.ExecuteIfBound(false, TEXT("新手村连接服务不可用。"), TEXT("{}"));
		return;
	}
	SessionService->GetConnection(SessionId, Callback);
}

void UDBABackendFacadeSubsystem::TrackEvent(const FString& EventName)
{
	UDBA_GameBackendClientSubsystem* Backend = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>()
		: nullptr;
	if (Backend && Backend->GetTelemetryService() && !EventName.IsEmpty())
	{
		Backend->GetTelemetryService()->TrackEvent(EventName, TMap<FString, FString>());
	}
}
