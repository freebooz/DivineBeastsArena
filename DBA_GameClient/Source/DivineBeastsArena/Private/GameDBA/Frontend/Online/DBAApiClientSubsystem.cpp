// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Online/DBAApiClientSubsystem.h"

#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "GameCore/Core/DBALogChannels.h"
#include "GameDBA/Frontend/Core/DBAFrontendErrorMapper.h"
#include "GameDBA/Frontend/Online/DBASecureTokenStorage.h"

void UDBAApiClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UDBA_GameBackendClientSubsystem>();
	Super::Initialize(Collection);
	TokenStorage = NewObject<UDBADevelopmentTokenStorage>(this);
	UE_LOG(LogDBAOnline, Log, TEXT("前台 ApiClient 已初始化：复用 GameBackendClient 作为唯一 HTTP 传输。"));
}

void UDBAApiClientSubsystem::Deinitialize()
{
	InvalidateSession();
	TokenStorage = nullptr;
	Super::Deinitialize();
}

bool UDBAApiClientSubsystem::IsSupportedInCurrentEnvironment() const
{
	return !IsRunningDedicatedServer();
}

FGuid UDBAApiClientSubsystem::Get(const FString& Path, const bool bRequiresAuthentication, UObject* CallbackOwner, FDBAApiCompletion Completion)
{
	FDBAApiRequest Request;
	Request.Verb = EDBAApiHttpVerb::Get;
	Request.Path = Path;
	Request.bRequiresAuthentication = bRequiresAuthentication;
	return Send(Request, CallbackOwner, MoveTemp(Completion));
}

FGuid UDBAApiClientSubsystem::Post(const FString& Path, const FString& JsonBody, const bool bRequiresAuthentication, UObject* CallbackOwner, FDBAApiCompletion Completion)
{
	FDBAApiRequest Request;
	Request.Verb = EDBAApiHttpVerb::Post;
	Request.Path = Path;
	Request.JsonBody = JsonBody;
	Request.bRequiresAuthentication = bRequiresAuthentication;
	return Send(Request, CallbackOwner, MoveTemp(Completion));
}

FGuid UDBAApiClientSubsystem::Put(const FString& Path, const FString& JsonBody, const bool bRequiresAuthentication, UObject* CallbackOwner, FDBAApiCompletion Completion)
{
	FDBAApiRequest Request;
	Request.Verb = EDBAApiHttpVerb::Put;
	Request.Path = Path;
	Request.JsonBody = JsonBody;
	Request.bRequiresAuthentication = bRequiresAuthentication;
	return Send(Request, CallbackOwner, MoveTemp(Completion));
}

FGuid UDBAApiClientSubsystem::Delete(const FString& Path, const bool bRequiresAuthentication, UObject* CallbackOwner, FDBAApiCompletion Completion)
{
	FDBAApiRequest Request;
	Request.Verb = EDBAApiHttpVerb::Delete;
	Request.Path = Path;
	Request.bRequiresAuthentication = bRequiresAuthentication;
	return Send(Request, CallbackOwner, MoveTemp(Completion));
}

FGuid UDBAApiClientSubsystem::Send(const FDBAApiRequest& InRequest, UObject* CallbackOwner, FDBAApiCompletion Completion)
{
	const FGuid RequestId = FGuid::NewGuid();
	FDBAApiRequest Request = InRequest;
	Request.Path.TrimStartAndEndInline();
	if (Request.CorrelationId.IsEmpty())
	{
		Request.CorrelationId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
	}

	FPendingRequest& Pending = PendingRequests.Add(RequestId);
	Pending.Request = MoveTemp(Request);
	Pending.CallbackOwner = CallbackOwner;
	Pending.Completion = MoveTemp(Completion);
	Pending.SessionGeneration = SessionGeneration;

	if (Pending.Request.Path.IsEmpty())
	{
		FDBAApiResponse InvalidResponse;
		InvalidResponse.CorrelationId = Pending.Request.CorrelationId;
		InvalidResponse.Result = FDBAOperationResult::Failure(UDBAFrontendErrorMapper::FromHttpStatus(400, TEXT("request.path_missing")));
		CompleteRequest(RequestId, InvalidResponse);
		return RequestId;
	}

	DispatchTransport(RequestId);
	return RequestId;
}

bool UDBAApiClientSubsystem::CancelRequest(const FGuid& RequestId)
{
	FPendingRequest* Pending = PendingRequests.Find(RequestId);
	if (!Pending)
	{
		return false;
	}

	if (UDBA_GameBackendClientSubsystem* Backend = GetBackendClient())
	{
		if (FDBA_GameBackendHttpClient* Transport = Backend->GetHttpClient())
		{
			Transport->CancelRequest(Pending->TransportRequestId);
		}
	}
	PendingRequests.Remove(RequestId);
	return true;
}

void UDBAApiClientSubsystem::CancelRequestsFor(const UObject* CallbackOwner)
{
	if (!CallbackOwner)
	{
		return;
	}

	TArray<FGuid> RequestIds;
	for (const TPair<FGuid, FPendingRequest>& Pair : PendingRequests)
	{
		if (Pair.Value.CallbackOwner.Get() == CallbackOwner)
		{
			RequestIds.Add(Pair.Key);
		}
	}
	for (const FGuid& RequestId : RequestIds)
	{
		CancelRequest(RequestId);
	}
}

void UDBAApiClientSubsystem::CancelOutstandingRequests()
{
	TArray<FGuid> RequestIds;
	PendingRequests.GetKeys(RequestIds);
	for (const FGuid& RequestId : RequestIds)
	{
		CancelRequest(RequestId);
	}
}

void UDBAApiClientSubsystem::SetAuthenticationTokens(const FString& AccessToken, const FString& RefreshToken, const FString& PlayerId)
{
	if (UDBA_GameBackendClientSubsystem* Backend = GetBackendClient())
	{
		Backend->SetAuthTokens(AccessToken, RefreshToken, PlayerId);
	}
	if (TokenStorage)
	{
		if (bPersistRefreshToken && !RefreshToken.IsEmpty())
		{
			TokenStorage->StoreRefreshToken(RefreshToken);
		}
		else
		{
			TokenStorage->ClearRefreshToken();
		}
	}
}

void UDBAApiClientSubsystem::SetRefreshTokenPersistenceEnabled(const bool bEnabled)
{
	bPersistRefreshToken = bEnabled;
	if (!bPersistRefreshToken && TokenStorage)
	{
		TokenStorage->ClearRefreshToken();
	}
}

bool UDBAApiClientSubsystem::LoadDevelopmentRefreshToken(FString& OutRefreshToken) const
{
	OutRefreshToken.Reset();
	return bPersistRefreshToken && TokenStorage && TokenStorage->LoadRefreshToken(OutRefreshToken);
}

void UDBAApiClientSubsystem::InvalidateSession()
{
	++SessionGeneration;
	bRefreshInFlight = false;
	CancelOutstandingRequests();
	if (TokenStorage)
	{
		TokenStorage->ClearRefreshToken();
	}
	if (UDBA_GameBackendClientSubsystem* Backend = GetBackendClient())
	{
		Backend->ClearAuthTokens();
	}
}

void UDBAApiClientSubsystem::SetMockTransportForTests(FDBAApiMockTransport InMockTransport)
{
	MockTransport = MoveTemp(InMockTransport);
}

void UDBAApiClientSubsystem::ClearMockTransportForTests()
{
	MockTransport = nullptr;
	MockRefresh = nullptr;
}

void UDBAApiClientSubsystem::SetMockRefreshForTests(FDBAApiMockRefresh InMockRefresh)
{
	MockRefresh = MoveTemp(InMockRefresh);
}

void UDBAApiClientSubsystem::DispatchTransport(const FGuid& RequestId)
{
	FPendingRequest* Pending = PendingRequests.Find(RequestId);
	if (!Pending || Pending->SessionGeneration != SessionGeneration || !Pending->CallbackOwner.IsValid())
	{
		CancelRequest(RequestId);
		return;
	}

	if (MockTransport)
	{
		const TWeakObjectPtr<UDBAApiClientSubsystem> WeakThis(this);
		MockTransport(Pending->Request, [WeakThis, RequestId](const FDBAApiResponse& Response)
		{
			if (UDBAApiClientSubsystem* ApiClient = WeakThis.Get())
			{
				ApiClient->HandleTransportResponse(RequestId, Response);
			}
		});
		return;
	}

	UDBA_GameBackendClientSubsystem* Backend = GetBackendClient();
	FDBA_GameBackendHttpClient* Transport = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Transport)
	{
		FDBAApiResponse Response;
		Response.CorrelationId = Pending->Request.CorrelationId;
		Response.Result = FDBAOperationResult::Failure(UDBAFrontendErrorMapper::FromHttpStatus(0, TEXT("network.transport_unavailable")));
		CompleteRequest(RequestId, Response);
		return;
	}

	FDBA_GameBackendHttpRequestOptions Options;
	Options.bRequiresAuth = Pending->Request.bRequiresAuthentication;
	Options.bAllowRefresh = false;
	Options.CorrelationId = Pending->Request.CorrelationId;
	Options.TimeoutSeconds = Pending->Request.TimeoutSeconds;
	Options.AdditionalHeaders = Pending->Request.Headers;
	const TWeakObjectPtr<UDBAApiClientSubsystem> WeakThis(this);
	const FGuid TransportRequestId = Transport->SendRequest(
		ToHttpVerb(Pending->Request.Verb),
		Pending->Request.Path,
		Pending->Request.JsonBody,
		Options,
		[WeakThis, RequestId](const FDBA_GameBackendHttpResult& TransportResult)
		{
			if (UDBAApiClientSubsystem* ApiClient = WeakThis.Get())
			{
				ApiClient->HandleTransportResponse(RequestId, ApiClient->MakeResponseFromTransportResult(TransportResult));
			}
		});
	if (FPendingRequest* CurrentPending = PendingRequests.Find(RequestId))
	{
		CurrentPending->TransportRequestId = TransportRequestId;
	}
}

void UDBAApiClientSubsystem::HandleTransportResponse(const FGuid& RequestId, const FDBAApiResponse& Response)
{
	FPendingRequest* Pending = PendingRequests.Find(RequestId);
	if (!Pending || Pending->SessionGeneration != SessionGeneration || !Pending->CallbackOwner.IsValid())
	{
		CancelRequest(RequestId);
		return;
	}

	const bool bCanRefresh = Pending->Request.bRequiresAuthentication
		&& Pending->Request.bRetryAfterAuthenticationRefresh
		&& !Pending->bRefreshAttempted
		&& Response.HttpStatusCode == 401;
	if (bCanRefresh)
	{
		Pending->bRefreshAttempted = true;
		Pending->TransportRequestId.Invalidate();
		QueueAuthenticationRefresh(RequestId);
		return;
	}

	CompleteRequest(RequestId, Response);
}

void UDBAApiClientSubsystem::QueueAuthenticationRefresh(const FGuid& RequestId)
{
	if (!PendingRequests.Contains(RequestId) || bRefreshInFlight)
	{
		return;
	}

	bRefreshInFlight = true;
	const TWeakObjectPtr<UDBAApiClientSubsystem> WeakThis(this);
	if (MockRefresh)
	{
		MockRefresh([WeakThis](const bool bSuccess)
		{
			if (UDBAApiClientSubsystem* ApiClient = WeakThis.Get())
			{
				ApiClient->HandleAuthenticationRefreshCompleted(bSuccess);
			}
		});
		return;
	}

	UDBA_GameBackendClientSubsystem* Backend = GetBackendClient();
	if (!Backend)
	{
		HandleAuthenticationRefreshCompleted(false);
		return;
	}

	Backend->RequestRefreshToken([WeakThis](const bool bSuccess)
	{
		if (UDBAApiClientSubsystem* ApiClient = WeakThis.Get())
		{
			ApiClient->HandleAuthenticationRefreshCompleted(bSuccess);
		}
	});
}

void UDBAApiClientSubsystem::HandleAuthenticationRefreshCompleted(const bool bSuccess)
{
	bRefreshInFlight = false;
	TArray<FGuid> WaitingRequestIds;
	for (const TPair<FGuid, FPendingRequest>& Pair : PendingRequests)
	{
		if (Pair.Value.bRefreshAttempted && !Pair.Value.TransportRequestId.IsValid())
		{
			WaitingRequestIds.Add(Pair.Key);
		}
	}

	for (const FGuid& RequestId : WaitingRequestIds)
	{
		FPendingRequest* Pending = PendingRequests.Find(RequestId);
		if (!Pending)
		{
			continue;
		}

		if (!bSuccess)
		{
			FDBAApiResponse Response;
			Response.CorrelationId = Pending->Request.CorrelationId;
			Response.HttpStatusCode = 401;
			Response.Result = FDBAOperationResult::Failure(UDBAFrontendErrorMapper::FromHttpStatus(401, TEXT("auth.refresh_failed")));
			CompleteRequest(RequestId, Response);
			continue;
		}

		Pending->TransportRequestId.Invalidate();
		DispatchTransport(RequestId);
	}
}

void UDBAApiClientSubsystem::CompleteRequest(const FGuid& RequestId, const FDBAApiResponse& Response)
{
	FPendingRequest Pending;
	if (!PendingRequests.RemoveAndCopyValue(RequestId, Pending))
	{
		return;
	}

	if (Pending.SessionGeneration != SessionGeneration || !Pending.CallbackOwner.IsValid())
	{
		return;
	}
	if (Pending.Completion)
	{
		Pending.Completion(Response);
	}
}

FDBAApiResponse UDBAApiClientSubsystem::MakeResponseFromTransportResult(const FDBA_GameBackendHttpResult& TransportResult) const
{
	FDBAApiResponse Response;
	Response.CorrelationId = TransportResult.TraceId;
	Response.HttpStatusCode = TransportResult.HttpStatus;
	if (TransportResult.IsSuccessful())
	{
		Response.Result = FDBAOperationResult::Success();
		Response.DomainJson = TransportResult.DataJson;
		return Response;
	}

	Response.Result = FDBAOperationResult::Failure(UDBAFrontendErrorMapper::FromHttpStatus(TransportResult.HttpStatus, FName(*TransportResult.Code)));
	return Response;
}

UDBA_GameBackendClientSubsystem* UDBAApiClientSubsystem::GetBackendClient() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr;
}

FString UDBAApiClientSubsystem::ToHttpVerb(const EDBAApiHttpVerb Verb)
{
	switch (Verb)
	{
	case EDBAApiHttpVerb::Get: return TEXT("GET");
	case EDBAApiHttpVerb::Post: return TEXT("POST");
	case EDBAApiHttpVerb::Put: return TEXT("PUT");
	case EDBAApiHttpVerb::Delete: return TEXT("DELETE");
	default: return TEXT("GET");
	}
}
