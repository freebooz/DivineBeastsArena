// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameCore/Account/DBAOnlineAccountService.h"

#include "GameCore/Account/DBAAccountSaveGame.h"
#include "GameCore/Account/DBAMockAccountService.h"
#include "GameCore/Account/DBAOnlineAccountJson.h"
#include "GameBackendClientSettings.h"
#include "GameBackendClientSubsystem.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
FString NormalizeBaseUrl(FString BaseUrl)
{
	BaseUrl.TrimStartAndEndInline();
	while (BaseUrl.EndsWith(TEXT("/")))
	{
		BaseUrl.LeftChopInline(1);
	}
	return BaseUrl;
}

FString BuildStableGuestDeviceId()
{
	FString DeviceId = FPlatformProcess::ComputerName();
	return DeviceId.IsEmpty() ? TEXT("DBA_LOCAL_DEVICE") : FString::Printf(TEXT("DBA_%s"), *DeviceId);
}

FString BuildRefreshTokenRequestBody(const FString& RefreshToken)
{
	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("refreshToken"), RefreshToken);

	FString Output;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Output);
	FJsonSerializer::Serialize(Object, Writer);
	return Output;
}
}

UDBAOnlineAccountService::UDBAOnlineAccountService()
{
}

void UDBAOnlineAccountService::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();

	MockService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAMockAccountService>() : nullptr;
	LoadOnlineAccountState();
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
	if (const UDBA_GameBackendClientSettings* BackendSettings = GetDefault<UDBA_GameBackendClientSettings>())
	{
		const FString BackendBaseUrl = NormalizeBaseUrl(BackendSettings->BackendBaseUrl);
		if (!BackendBaseUrl.IsEmpty())
		{
			return BackendBaseUrl + Path;
		}
	}

	return NormalizeBaseUrl(OnlineConfig.GetBaseUrl()) + Path;
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
	RefreshToken = Response.RefreshToken;
	if (UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr)
	{
		const FString BackendPlayerId = Response.PlayerId.IsEmpty() ? Response.AccountInfo.AccountId.ToString() : Response.PlayerId;
		Backend->SetAuthTokens(Response.SessionToken, Response.RefreshToken, BackendPlayerId);
	}
	SaveOnlineAccountState();
}

void UDBAOnlineAccountService::LoadOnlineAccountState()
{
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (!SaveGame || !SaveGame->IsValid())
	{
		return;
	}

	CurrentAccountInfo = SaveGame->AccountInfo;
	CurrentCharacterId = SaveGame->CurrentCharacterId;
	SessionToken = SaveGame->SessionToken;
	RefreshToken = SaveGame->RefreshToken;

	if (!SessionToken.IsEmpty())
	{
		if (UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr)
		{
			Backend->SetAuthTokens(SessionToken, RefreshToken, CurrentAccountInfo.AccountId.ToString());
		}
	}
}

void UDBAOnlineAccountService::SaveOnlineAccountState(const TArray<FDBACharacterSummary>* Characters)
{
	UDBAAccountSaveGame* SaveGame = LoadAccountSaveGame();
	if (!SaveGame)
	{
		SaveGame = CreateDefaultAccountSaveGame();
	}
	if (!SaveGame)
	{
		return;
	}

	SaveGame->AccountInfo = CurrentAccountInfo;
	SaveGame->CurrentCharacterId = CurrentCharacterId;
	SaveGame->SessionToken = SessionToken;
	SaveGame->RefreshToken = RefreshToken;
	if (Characters)
	{
		SaveGame->Characters = *Characters;
	}
	SaveAccountSaveGame(SaveGame);
}

bool UDBAOnlineAccountService::ShouldFallback(EDBAOnlineAccountError Error) const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return OnlineConfig.bAllowMockFallback && CanFallbackToMock(Error) && MockService;
#endif
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
	if (!EnsureGameThread(TEXT("GuestLogin")))
	{
		return;
	}

	FDBALoginRequest RequestData;
	RequestData.LoginType = EDBALoginType::Guest;
	RequestData.DeviceId = BuildStableGuestDeviceId();

	const FString Body = FDBAOnlineAccountJson::BuildLoginRequest(RequestData);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateJsonRequest(TEXT("POST"), TEXT("/api/auth/guest-login"), Body);
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
			Response.ErrorMessage = TEXT("游客登录失败，请先启动 DBA_GameBackend 服务端。");
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

void UDBAOnlineAccountService::AutoLogin(FDBAOnLoginComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("AutoLogin")))
	{
		return;
	}

	if (RefreshToken.IsEmpty())
	{
		LoadOnlineAccountState();
	}

	if (RefreshToken.IsEmpty())
	{
		if (OnlineConfig.bAllowMockFallback)
		{
			FallbackAutoLogin(OnComplete);
			return;
		}

		FDBALoginResponse Response;
		Response.ErrorMessage = TEXT("没有可刷新的登录令牌，请先通过服务端登录。");
		OnComplete.ExecuteIfBound(Response);
		return;
	}

	const FString Body = BuildRefreshTokenRequestBody(RefreshToken);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateJsonRequest(TEXT("POST"), TEXT("/api/auth/refresh"), Body);
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
			if (Response.PlayerId.IsEmpty() && CurrentAccountInfo.IsValid())
			{
				Response.AccountInfo = CurrentAccountInfo;
				Response.PlayerId = CurrentAccountInfo.AccountId.ToString();
			}
			CacheLoginSuccess(Response);
			OnComplete.ExecuteIfBound(Response);
			return;
		}

		Response.ErrorMessage = Error.IsEmpty() ? TEXT("Auto login failed") : Error;
		OnComplete.ExecuteIfBound(Response);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::Logout(FDBAOnLogoutComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("Logout")))
	{
		return;
	}

	const auto FinishLogout = [this, OnComplete]()
	{
		CurrentAccountInfo = FDBAAccountInfo();
		CurrentCharacterId = FDBACharacterId();
		SessionToken.Empty();
		RefreshToken.Empty();
		if (UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr)
		{
			Backend->ClearAuthTokens();
		}
		SaveOnlineAccountState();
		OnComplete.ExecuteIfBound();
	};

	if (SessionToken.IsEmpty())
	{
		FinishLogout();
		return;
	}

	const FString Body = RefreshToken.IsEmpty() ? TEXT("{}") : BuildRefreshTokenRequestBody(RefreshToken);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateJsonRequest(TEXT("POST"), TEXT("/api/auth/logout"), Body);
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, FinishLogout](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		PendingRequests.Remove(HttpRequest);
		FinishLogout();
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
		SaveOnlineAccountState(&Characters);
		OnComplete.ExecuteIfBound(Characters);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::CreateCharacter(const FDBACharacterCreateRequest& RequestData, FDBAOnCharacterCreated OnComplete)
{
	FString ValidationError;
	if (!ValidateCharacterName(RequestData.CharacterName, ValidationError))
	{
		FDBACharacterCreateResponse Response;
		Response.ErrorMessage = ValidationError;
		OnComplete.ExecuteIfBound(Response);
		return;
	}

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
		if (Response.bSuccess)
		{
			SaveOnlineAccountState();
		}
		OnComplete.ExecuteIfBound(Response);
	});
	Request->ProcessRequest();
}

void UDBAOnlineAccountService::SelectCharacter(const FDBACharacterId& CharacterId, FDBAOnCharacterSelected OnComplete)
{
	if (!EnsureGameThread(TEXT("SelectCharacter")))
	{
		return;
	}

	if (!CharacterId.IsValid())
	{
		OnComplete.ExecuteIfBound(FDBACharacterId());
		return;
	}

	const FString Body = FDBAOnlineAccountJson::BuildSelectCharacterRequest(CharacterId);
	const FString Path = FString::Printf(TEXT("/api/account/characters/%s/select"), *CharacterId.ToString());
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateJsonRequest(TEXT("POST"), Path, Body);
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, CharacterId, OnComplete](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		PendingRequests.Remove(HttpRequest);

		if (!bSucceeded || !HttpResponse.IsValid() || !EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			OnComplete.ExecuteIfBound(FDBACharacterId());
			return;
		}

		FDBACharacterId SelectedId;
		FString Error;
		if (!FDBAOnlineAccountJson::ParseSelectCharacterResponse(HttpResponse->GetContentAsString(), SelectedId, Error) || !SelectedId.IsValid())
		{
			LogSubsystemWarning(Error.IsEmpty() ? TEXT("Online character selection failed") : Error);
			OnComplete.ExecuteIfBound(FDBACharacterId());
			return;
		}

		CurrentCharacterId = SelectedId;
		SaveOnlineAccountState();
		OnComplete.ExecuteIfBound(SelectedId);
	});
	Request->ProcessRequest();
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
