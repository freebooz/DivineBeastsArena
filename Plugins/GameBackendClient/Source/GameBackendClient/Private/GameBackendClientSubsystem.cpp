// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendClientSubsystem.h"

#include "GameBackendAuthService.h"
#include "GameBackendPlayerService.h"
#include "GameBackendConfigService.h"
#include "GameBackendRoomService.h"
#include "GameBackendMatchService.h"
#include "GameBackendSessionService.h"
#include "GameBackendMailService.h"
#include "GameBackendSupportService.h"
#include "GameBackendTelemetryService.h"
#include "GameBackendCrashService.h"
#include "GameBackendHttpClient.h"
#include "GameBackendClientSettings.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	bool ExtractTokensFromData(const FString& DataJson, FString& OutAccessToken, FString& OutRefreshToken, FString& OutPlayerId)
	{
		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DataJson);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			return false;
		}

		const auto ReadFirst = [&Root](const TArray<FString>& Keys) -> FString
		{
			for (const FString& Key : Keys)
			{
				FString Value;
				if (Root->TryGetStringField(Key, Value))
				{
					return Value;
				}
			}
			return FString();
		};

		OutAccessToken = ReadFirst({ TEXT("accessToken"), TEXT("access_token"), TEXT("token") });
		OutRefreshToken = ReadFirst({ TEXT("refreshToken"), TEXT("refresh_token") });
		OutPlayerId = ReadFirst({ TEXT("playerId"), TEXT("player_id"), TEXT("uid") });
		return !OutAccessToken.IsEmpty();
	}
}

void UGameBackendClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UGameBackendClientSettings* Settings = GetDefault<UGameBackendClientSettings>();
	BackendBaseUrl = Settings->BackendBaseUrl;
	ClientVersion = Settings->ClientVersion;
	BuildNumber = Settings->BuildNumber;
	Channel = Settings->Channel;
	Platform = Settings->Platform;
	Region = Settings->Region;
	RequestTimeoutSeconds = Settings->RequestTimeoutSeconds;
	HttpRetryCount = Settings->HttpRetryCount;

	HttpClient = MakeShared<FGameBackendHttpClient>(this);
	InitializeServices();

	UE_LOG(LogGameBackendClient, Log, TEXT("后端子系统初始化完成。BaseUrl=%s version=%s channel=%s platform=%s"),
		*BackendBaseUrl, *ClientVersion, *Channel, *Platform);

	if (CrashService)
	{
		CrashService->ScanCrashFiles();
	}

	if (TelemetryService)
	{
		TMap<FString, FString> Props;
		Props.Add(TEXT("version"), ClientVersion);
		Props.Add(TEXT("build"), BuildNumber);
		Props.Add(TEXT("channel"), Channel);
		Props.Add(TEXT("platform"), Platform);
		TelemetryService->TrackEvent(TEXT("client_started"), Props);
	}
}

void UGameBackendClientSubsystem::Deinitialize()
{
	if (TelemetryService)
	{
		TelemetryService->Flush();
		TelemetryService->Shutdown();
	}

	ReleaseServices();
	HttpClient.Reset();
	Super::Deinitialize();
}

bool UGameBackendClientSubsystem::IsLoggedIn() const
{
	return !AccessToken.IsEmpty() && !PlayerId.IsEmpty();
}

void UGameBackendClientSubsystem::Logout()
{
	if (AuthService)
	{
		FGameBackendResponseDelegate Dummy;
		AuthService->Logout(Dummy);
	}

	ClearAuthTokens();
}

FString UGameBackendClientSubsystem::GetAccessToken() const
{
	return AccessToken;
}

FString UGameBackendClientSubsystem::GetRefreshToken() const
{
	return RefreshToken;
}

FString UGameBackendClientSubsystem::GetPlayerId() const
{
	return PlayerId;
}

void UGameBackendClientSubsystem::SetAuthTokens(const FString& InAccessToken, const FString& InRefreshToken, const FString& InPlayerId)
{
	AccessToken = InAccessToken;
	RefreshToken = InRefreshToken;
	PlayerId = InPlayerId;
	UE_LOG(LogGameBackendClient, Log, TEXT("Auth tokens updated. playerId=%s"), *PlayerId);
}

void UGameBackendClientSubsystem::ClearAuthTokens()
{
	AccessToken.Empty();
	RefreshToken.Empty();
	PlayerId.Empty();
	UE_LOG(LogGameBackendClient, Log, TEXT("Auth tokens cleared."));
}

void UGameBackendClientSubsystem::TestVersionCheck(const FGameBackendResponseDelegate& Callback)
{
	if (ConfigService)
	{
		ConfigService->VersionCheck(Callback);
	}
}

void UGameBackendClientSubsystem::TestDevLogin(const FString& DisplayName, const FGameBackendAuthResponseDelegate& Callback)
{
	if (AuthService)
	{
		AuthService->DevLogin(DisplayName, Callback);
	}
}

void UGameBackendClientSubsystem::TestGetProfile(const FGameBackendResponseDelegate& Callback)
{
	if (PlayerService)
	{
		PlayerService->GetMyProfile(Callback);
	}
}

void UGameBackendClientSubsystem::TestGetConfigBundle(const FGameBackendResponseDelegate& Callback)
{
	if (ConfigService)
	{
		ConfigService->GetConfigBundle(Callback);
	}
}

void UGameBackendClientSubsystem::TestCreateRoom(const FGameBackendResponseDelegate& Callback)
{
	if (RoomService)
	{
		FGameBackendRoomCreateRequest Req;
		RoomService->CreateRoom(Req, Callback);
	}
}

void UGameBackendClientSubsystem::TestMatchmaking(const FString& Mode, const FString& RegionCode, const FGameBackendResponseDelegate& Callback)
{
	if (MatchService)
	{
		MatchService->CreateTicket(Mode, RegionCode, Callback);
	}
}

void UGameBackendClientSubsystem::TestTelemetry()
{
	if (TelemetryService)
	{
		TMap<FString, FString> Props;
		Props.Add(TEXT("source"), TEXT("TestTelemetry"));
		TelemetryService->TrackEvent(TEXT("telemetry_test"), Props);
		TelemetryService->Flush();
	}
}

FGameBackendHttpClient* UGameBackendClientSubsystem::GetHttpClient() const
{
	return HttpClient.Get();
}

float UGameBackendClientSubsystem::GetRequestTimeoutSeconds() const
{
	return RequestTimeoutSeconds;
}

int32 UGameBackendClientSubsystem::GetHttpRetryCount() const
{
	return HttpRetryCount;
}

void UGameBackendClientSubsystem::QueueRefreshCallback(TFunction<void(bool)> Completion)
{
	if (Completion)
	{
		RefreshCallbacks.Add(MoveTemp(Completion));
	}
}

void UGameBackendClientSubsystem::RequestRefreshToken(TFunction<void(bool)> Completion)
{
	QueueRefreshCallback(MoveTemp(Completion));
	if (bRefreshingToken)
	{
		return;
	}

	if (RefreshToken.IsEmpty() || !HttpClient.IsValid())
	{
		NotifyRefreshCompleted(false);
		return;
	}

	bRefreshingToken = true;

	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("refreshToken"), RefreshToken);

	FString Body;
	auto Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	HttpClient->Post(TEXT("/api/auth/refresh"), Body, [this](const FGameBackendHttpResult& Result)
	{
		bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		if (bSuccess)
		{
			FString NewAccessToken;
			FString NewRefreshToken;
			FString NewPlayerId;
			bSuccess = ExtractTokensFromData(Result.DataJson, NewAccessToken, NewRefreshToken, NewPlayerId);
			if (bSuccess)
			{
				SetAuthTokens(NewAccessToken, NewRefreshToken.IsEmpty() ? RefreshToken : NewRefreshToken, NewPlayerId.IsEmpty() ? PlayerId : NewPlayerId);
			}
		}

		NotifyRefreshCompleted(bSuccess);
	}, false);
}

void UGameBackendClientSubsystem::InitializeServices()
{
	AuthService = NewObject<UGameBackendAuthService>(this);
	PlayerService = NewObject<UGameBackendPlayerService>(this);
	ConfigService = NewObject<UGameBackendConfigService>(this);
	RoomService = NewObject<UGameBackendRoomService>(this);
	MatchService = NewObject<UGameBackendMatchService>(this);
	SessionService = NewObject<UGameBackendSessionService>(this);
	MailService = NewObject<UGameBackendMailService>(this);
	SupportService = NewObject<UGameBackendSupportService>(this);
	TelemetryService = NewObject<UGameBackendTelemetryService>(this);
	CrashService = NewObject<UGameBackendCrashService>(this);

	if (AuthService) AuthService->Initialize(this, HttpClient.Get());
	if (PlayerService) PlayerService->Initialize(this, HttpClient.Get());
	if (ConfigService) ConfigService->Initialize(this, HttpClient.Get());
	if (RoomService) RoomService->Initialize(this, HttpClient.Get());
	if (MatchService) MatchService->Initialize(this, HttpClient.Get());
	if (SessionService) SessionService->Initialize(this, HttpClient.Get());
	if (MailService) MailService->Initialize(this, HttpClient.Get());
	if (SupportService) SupportService->Initialize(this, HttpClient.Get());
	if (TelemetryService) TelemetryService->Initialize(this, HttpClient.Get());
	if (CrashService) CrashService->Initialize(this, HttpClient.Get());
}

void UGameBackendClientSubsystem::ReleaseServices()
{
	AuthService = nullptr;
	PlayerService = nullptr;
	ConfigService = nullptr;
	RoomService = nullptr;
	MatchService = nullptr;
	SessionService = nullptr;
	MailService = nullptr;
	SupportService = nullptr;
	TelemetryService = nullptr;
	CrashService = nullptr;
}

void UGameBackendClientSubsystem::NotifyRefreshCompleted(bool bSuccess)
{
	bRefreshingToken = false;
	TArray<TFunction<void(bool)>> Callbacks = MoveTemp(RefreshCallbacks);
	RefreshCallbacks.Reset();
	for (TFunction<void(bool)>& Callback : Callbacks)
	{
		if (Callback)
		{
			Callback(bSuccess);
		}
	}
}
