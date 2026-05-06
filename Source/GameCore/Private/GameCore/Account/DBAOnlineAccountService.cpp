// Copyright FreeboozStudio. All Rights Reserved.

#include "GameCore/Account/DBAOnlineAccountService.h"

#include "GameCore/Account/DBAMockAccountService.h"
#include "GameCore/Account/DBAOnlineAccountJson.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

UDBAOnlineAccountService::UDBAOnlineAccountService()
{
}

void UDBAOnlineAccountService::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();

	MockService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAMockAccountService>() : nullptr;
	LogSubsystemInfo(FString::Printf(TEXT("Online account service initialized: %s"), *OnlineConfig.GetBaseUrl()));
}

void UDBAOnlineAccountService::CancelAllAsyncOperations()
{
	for (const TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>& Request : PendingRequests)
	{
		if (Request.IsValid())
		{
			Request->CancelRequest();
		}
	}
	PendingRequests.Reset();
}

bool UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError Error)
{
	return Error == EDBAOnlineAccountError::NetworkUnavailable
		|| Error == EDBAOnlineAccountError::Timeout
		|| Error == EDBAOnlineAccountError::EndpointMissing
		|| Error == EDBAOnlineAccountError::ServiceUnavailable;
}

FString UDBAOnlineAccountService::BuildUrl(const FString& Path) const
{
	return OnlineConfig.GetBaseUrl() + Path;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UDBAOnlineAccountService::CreateJsonRequest(const FString& Verb, const FString& Path, const FString& Body)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb(Verb);
	Request->SetURL(BuildUrl(Path));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	if (!SessionToken.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *SessionToken));
	}
	if (!Body.IsEmpty())
	{
		Request->SetContentAsString(Body);
	}
	Request->SetTimeout(OnlineConfig.RequestTimeoutSeconds);
	PendingRequests.Add(Request);
	return Request;
}

EDBAOnlineAccountError UDBAOnlineAccountService::ClassifyHttpFailure(bool bSucceeded, const TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>& Response) const
{
	if (!bSucceeded || !Response.IsValid())
	{
		return EDBAOnlineAccountError::NetworkUnavailable;
	}

	const int32 Code = Response->GetResponseCode();
	if (Code == 401 || Code == 403)
	{
		return EDBAOnlineAccountError::InvalidCredentials;
	}
	if (Code == 404)
	{
		return EDBAOnlineAccountError::EndpointMissing;
	}
	if (Code == 408)
	{
		return EDBAOnlineAccountError::Timeout;
	}
	if (Code == 422)
	{
		return EDBAOnlineAccountError::ValidationFailed;
	}
	if (Code >= 500)
	{
		return EDBAOnlineAccountError::ServiceUnavailable;
	}

	return EDBAOnlineAccountError::MalformedResponse;
}

void UDBAOnlineAccountService::CacheLoginSuccess(const FDBALoginResponse& Response)
{
	CurrentAccountInfo = Response.AccountInfo;
	SessionToken = Response.SessionToken;
}

bool UDBAOnlineAccountService::ShouldFallback(EDBAOnlineAccountError Error) const
{
	return OnlineConfig.bAllowMockFallback && CanFallbackToMock(Error) && MockService;
}

void UDBAOnlineAccountService::Login(const FDBALoginRequest& RequestData, FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Login")))
	{
		return;
	}

	if (RequestData.LoginType == EDBALoginType::Guest)
	{
		GuestLogin(OnComplete);
		return;
	}

	const FString Body = FDBAOnlineAccountJson::BuildLoginRequest(RequestData);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateJsonRequest(TEXT("POST"), TEXT("/api/auth/login"), Body);
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, OnComplete](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		PendingRequests.Remove(HttpRequest);

		if (!bSucceeded || !HttpResponse.IsValid() || !EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			const EDBAOnlineAccountError Error = ClassifyHttpFailure(bSucceeded, HttpResponse);
			if (ShouldFallback(Error))
			{
				FallbackLogin(OnComplete);
				return;
			}

			FDBALoginResponse Response;
			Response.ErrorMessage = TEXT("Online login failed");
			OnComplete.ExecuteIfBound(Response);
			return;
		}

		FDBALoginResponse Response;
		FString Error;
		if (!FDBAOnlineAccountJson::ParseLoginResponse(HttpResponse->GetContentAsString(), Response, Error))
		{
			Response.ErrorMessage = Error;
			OnComplete.ExecuteIfBound(Response);
			return;
		}

		if (Response.bSuccess)
		{
			CacheLoginSuccess(Response);
		}
		OnComplete.ExecuteIfBound(Response);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::Register(const FDBALoginRequest& RequestData, FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Register")))
	{
		return;
	}

	const FString Body = FDBAOnlineAccountJson::BuildLoginRequest(RequestData);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateJsonRequest(TEXT("POST"), TEXT("/api/auth/register"), Body);
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, OnComplete](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		PendingRequests.Remove(HttpRequest);

		if (!bSucceeded || !HttpResponse.IsValid() || !EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			FDBALoginResponse Response;
			Response.ErrorMessage = TEXT("Online registration failed");
			OnComplete.ExecuteIfBound(Response);
			return;
		}

		FDBALoginResponse Response;
		FString Error;
		if (!FDBAOnlineAccountJson::ParseLoginResponse(HttpResponse->GetContentAsString(), Response, Error))
		{
			Response.ErrorMessage = Error;
			OnComplete.ExecuteIfBound(Response);
			return;
		}

		if (Response.bSuccess)
		{
			CacheLoginSuccess(Response);
		}
		OnComplete.ExecuteIfBound(Response);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::GuestLogin(FDBAOnLoginComplete OnComplete)
{
	FallbackLogin(OnComplete);
}

void UDBAOnlineAccountService::AutoLogin(FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("AutoLogin")))
	{
		return;
	}

	if (SessionToken.IsEmpty())
	{
		FallbackAutoLogin(OnComplete);
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateJsonRequest(TEXT("POST"), TEXT("/api/auth/refresh"), TEXT("{}"));
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, OnComplete](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		PendingRequests.Remove(HttpRequest);

		if (!bSucceeded || !HttpResponse.IsValid() || !EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			const EDBAOnlineAccountError Error = ClassifyHttpFailure(bSucceeded, HttpResponse);
			if (ShouldFallback(Error))
			{
				FallbackAutoLogin(OnComplete);
				return;
			}
		}

		FDBALoginResponse Response;
		FString Error;
		if (HttpResponse.IsValid() && FDBAOnlineAccountJson::ParseLoginResponse(HttpResponse->GetContentAsString(), Response, Error) && Response.bSuccess)
		{
			CacheLoginSuccess(Response);
			OnComplete.ExecuteIfBound(Response);
			return;
		}

		Response.ErrorMessage = Error.IsEmpty() ? TEXT("Auto login failed") : Error;
		OnComplete.ExecuteIfBound(Response);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::GetCharacterList(FDBAOnCharacterListLoaded OnComplete)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateJsonRequest(TEXT("GET"), TEXT("/api/account/characters"), TEXT(""));
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, OnComplete](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		PendingRequests.Remove(HttpRequest);

		if (!bSucceeded || !HttpResponse.IsValid() || !EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			const EDBAOnlineAccountError Error = ClassifyHttpFailure(bSucceeded, HttpResponse);
			if (ShouldFallback(Error))
			{
				FallbackCharacterList(OnComplete);
				return;
			}

			TArray<FDBACharacterSummary> Empty;
			OnComplete.ExecuteIfBound(Empty);
			return;
		}

		TArray<FDBACharacterSummary> Characters;
		FString Error;
		if (!FDBAOnlineAccountJson::ParseCharacterListResponse(HttpResponse->GetContentAsString(), Characters, Error))
		{
			LogSubsystemWarning(Error);
		}
		OnComplete.ExecuteIfBound(Characters);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::CreateCharacter(const FDBACharacterCreateRequest& RequestData, FDBAOnCharacterCreated OnComplete)
{
	const FString Body = FDBAOnlineAccountJson::BuildCreateCharacterRequest(RequestData);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateJsonRequest(TEXT("POST"), TEXT("/api/account/characters"), Body);
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, RequestData, OnComplete](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		PendingRequests.Remove(HttpRequest);

		if (!bSucceeded || !HttpResponse.IsValid() || !EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			const EDBAOnlineAccountError Error = ClassifyHttpFailure(bSucceeded, HttpResponse);
			if (ShouldFallback(Error))
			{
				FallbackCreateCharacter(RequestData, OnComplete);
				return;
			}

			FDBACharacterCreateResponse Response;
			Response.ErrorMessage = TEXT("Online character creation failed");
			OnComplete.ExecuteIfBound(Response);
			return;
		}

		FDBACharacterCreateResponse Response;
		FString Error;
		if (!FDBAOnlineAccountJson::ParseCreateCharacterResponse(HttpResponse->GetContentAsString(), Response, Error))
		{
			Response.ErrorMessage = Error;
		}
		OnComplete.ExecuteIfBound(Response);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::SelectCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterSelected OnComplete)
{
	CurrentCharacterId = CharacterId;
	OnComplete.ExecuteIfBound(CharacterId);
}

void UDBAOnlineAccountService::FallbackLogin(FDBAOnLoginComplete OnComplete)
{
	if (!MockService)
	{
		FDBALoginResponse Response;
		Response.ErrorMessage = TEXT("Mock fallback unavailable");
		OnComplete.ExecuteIfBound(Response);
		return;
	}
	LogSubsystemWarning(TEXT("Online login unavailable, fallback to mock guest login"));
	MockService->GuestLogin(FDBAOnLoginComplete::CreateWeakLambda(this, [this, OnComplete](const FDBALoginResponse& Response)
	{
		if (Response.bSuccess)
		{
			CacheLoginSuccess(Response);
		}
		OnComplete.ExecuteIfBound(Response);
	}));
}

void UDBAOnlineAccountService::FallbackAutoLogin(FDBAOnLoginComplete OnComplete)
{
	if (MockService)
	{
		MockService->AutoLogin(FDBAOnLoginComplete::CreateWeakLambda(this, [this, OnComplete](const FDBALoginResponse& Response)
		{
			if (Response.bSuccess)
			{
				CacheLoginSuccess(Response);
			}
			OnComplete.ExecuteIfBound(Response);
		}));
		return;
	}

	FDBALoginResponse Response;
	Response.ErrorMessage = TEXT("Mock fallback unavailable");
	OnComplete.ExecuteIfBound(Response);
}

void UDBAOnlineAccountService::FallbackCharacterList(FDBAOnCharacterListLoaded OnComplete)
{
	if (MockService)
	{
		MockService->GetCharacterList(OnComplete);
		return;
	}

	TArray<FDBACharacterSummary> Empty;
	OnComplete.ExecuteIfBound(Empty);
}

void UDBAOnlineAccountService::FallbackCreateCharacter(const FDBACharacterCreateRequest& Request, FDBAOnCharacterCreated OnComplete)
{
	if (MockService)
	{
		MockService->CreateCharacter(Request, OnComplete);
		return;
	}

	FDBACharacterCreateResponse Response;
	Response.ErrorMessage = TEXT("Mock fallback unavailable");
	OnComplete.ExecuteIfBound(Response);
}
