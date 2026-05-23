// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient / GameBackendClient Unreal 插件。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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
#include "GameBackendRuntimeService.h"
#include "GameBackendHttpClient.h"
#include "GameBackendClientSettings.h"
#include "Dom/JsonObject.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
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

	FString ReadCommandLineOverride(const TArray<const TCHAR*>& Keys)
	{
		for (const TCHAR* Key : Keys)
		{
			FString Value;
			if (FParse::Value(FCommandLine::Get(), Key, Value))
			{
				Value.TrimStartAndEndInline();
				if (!Value.IsEmpty())
				{
					return Value;
				}
			}
		}
		return FString();
	}
}

void UDBA_GameBackendClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UDBA_GameBackendClientSettings* Settings = GetDefault<UDBA_GameBackendClientSettings>();
	BackendBaseUrl = Settings->BackendBaseUrl;
	const FString BackendUrlOverride = ReadCommandLineOverride({ TEXT("backendUrl="), TEXT("BackendBaseUrl="), TEXT("DBABackendUrl=") });
	if (!BackendUrlOverride.IsEmpty())
	{
		BackendBaseUrl = BackendUrlOverride;
	}
	ClientVersion = Settings->ClientVersion;
	BuildNumber = Settings->BuildNumber;
	Channel = Settings->Channel;
	Platform = Settings->Platform;
	Region = Settings->Region;
	RequestTimeoutSeconds = Settings->RequestTimeoutSeconds;
	HttpRetryCount = Settings->HttpRetryCount;

	HttpClient = MakeShared<FDBA_GameBackendHttpClient>(this);
	InitializeServices();

	UE_LOG(LogDBA_GameBackendClient, Log, TEXT("后端子系统初始化完成。基础地址=%s 版本=%s 渠道=%s 平台=%s"),
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

void UDBA_GameBackendClientSubsystem::Deinitialize()
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

bool UDBA_GameBackendClientSubsystem::IsLoggedIn() const
{
	return !AccessToken.IsEmpty() && !PlayerId.IsEmpty();
}

void UDBA_GameBackendClientSubsystem::Logout()
{
	if (AuthService)
	{
		FDBA_GameBackendResponseDelegate Dummy;
		AuthService->Logout(Dummy);
	}

	ClearAuthTokens();
}

FString UDBA_GameBackendClientSubsystem::GetAccessToken() const
{
	return AccessToken;
}

FString UDBA_GameBackendClientSubsystem::GetRefreshToken() const
{
	return RefreshToken;
}

FString UDBA_GameBackendClientSubsystem::GetPlayerId() const
{
	return PlayerId;
}

void UDBA_GameBackendClientSubsystem::SetAuthTokens(const FString& InAccessToken, const FString& InRefreshToken, const FString& InPlayerId)
{
	AccessToken = InAccessToken;
	RefreshToken = InRefreshToken;
	PlayerId = InPlayerId;
	UE_LOG(LogDBA_GameBackendClient, Log, TEXT("鉴权令牌已更新。玩家ID=%s"), *PlayerId);
}

void UDBA_GameBackendClientSubsystem::ClearAuthTokens()
{
	AccessToken.Empty();
	RefreshToken.Empty();
	PlayerId.Empty();
	UE_LOG(LogDBA_GameBackendClient, Log, TEXT("鉴权令牌已清除。"));
}

void UDBA_GameBackendClientSubsystem::TestVersionCheck(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (ConfigService)
	{
		ConfigService->VersionCheck(Callback);
	}
}

void UDBA_GameBackendClientSubsystem::TestDevLogin(const FString& DisplayName, const FDBA_GameBackendAuthResponseDelegate& Callback)
{
	if (AuthService)
	{
		AuthService->DevLogin(DisplayName, Callback);
	}
}

void UDBA_GameBackendClientSubsystem::TestGetProfile(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (PlayerService)
	{
		PlayerService->GetMyProfile(Callback);
	}
}

void UDBA_GameBackendClientSubsystem::TestGetConfigBundle(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (ConfigService)
	{
		ConfigService->GetConfigBundle(Callback);
	}
}

void UDBA_GameBackendClientSubsystem::TestCreateRoom(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (RoomService)
	{
		FDBA_GameBackendRoomCreateRequest Req;
		RoomService->CreateRoom(Req, Callback);
	}
}

void UDBA_GameBackendClientSubsystem::TestMatchmaking(const FString& Mode, const FString& RegionCode, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (MatchService)
	{
		MatchService->CreateTicket(Mode, RegionCode, Callback);
	}
}

void UDBA_GameBackendClientSubsystem::TestTelemetry()
{
	if (TelemetryService)
	{
		TMap<FString, FString> Props;
		Props.Add(TEXT("source"), TEXT("TestTelemetry"));
		TelemetryService->TrackEvent(TEXT("telemetry_test"), Props);
		TelemetryService->Flush();
	}
}

FDBA_GameBackendHttpClient* UDBA_GameBackendClientSubsystem::GetHttpClient() const
{
	return HttpClient.Get();
}

float UDBA_GameBackendClientSubsystem::GetRequestTimeoutSeconds() const
{
	return RequestTimeoutSeconds;
}

int32 UDBA_GameBackendClientSubsystem::GetHttpRetryCount() const
{
	return HttpRetryCount;
}

void UDBA_GameBackendClientSubsystem::QueueRefreshCallback(TFunction<void(bool)> Completion)
{
	if (Completion)
	{
		RefreshCallbacks.Add(MoveTemp(Completion));
	}
}

void UDBA_GameBackendClientSubsystem::RequestRefreshToken(TFunction<void(bool)> Completion)
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

	HttpClient->Post(TEXT("/api/auth/refresh"), Body, [this](const FDBA_GameBackendHttpResult& Result)
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

void UDBA_GameBackendClientSubsystem::InitializeServices()
{
	AuthService = NewObject<UDBA_GameBackendAuthService>(this);
	PlayerService = NewObject<UDBA_GameBackendPlayerService>(this);
	ConfigService = NewObject<UDBA_GameBackendConfigService>(this);
	RoomService = NewObject<UDBA_GameBackendRoomService>(this);
	MatchService = NewObject<UDBA_GameBackendMatchService>(this);
	SessionService = NewObject<UDBA_GameBackendSessionService>(this);
	MailService = NewObject<UDBA_GameBackendMailService>(this);
	SupportService = NewObject<UDBA_GameBackendSupportService>(this);
	TelemetryService = NewObject<UDBA_GameBackendTelemetryService>(this);
	CrashService = NewObject<UDBA_GameBackendCrashService>(this);
	RuntimeService = NewObject<UDBA_GameBackendRuntimeService>(this);

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
	if (RuntimeService) RuntimeService->Initialize(this, HttpClient.Get());
}

void UDBA_GameBackendClientSubsystem::ReleaseServices()
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
	RuntimeService = nullptr;
}

void UDBA_GameBackendClientSubsystem::NotifyRefreshCompleted(bool bSuccess)
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
