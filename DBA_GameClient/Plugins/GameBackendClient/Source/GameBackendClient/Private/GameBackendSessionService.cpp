// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "DBA_GameBackendSessionService.h"

#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "DBA_GameBackendClientSubsystem.h"
#include "DBA_GameBackendHttpClient.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	void AppendTravelOption(FString& Url, const FString& Key, const FString& Value)
	{
		if (Value.IsEmpty())
		{
			return;
		}

		Url += Url.Contains(TEXT("?")) ? TEXT("&") : TEXT("?");
		Url += FString::Printf(TEXT("%s=%s"), *Key, *FGenericPlatformHttp::UrlEncode(Value));
	}

	void ExecuteResponse(const FDBA_GameBackendResponseDelegate& Callback, const FDBA_GameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.IsSuccessful();
		const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("Request failed.") : Result.Message);
		Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
	}
}

void UDBA_GameBackendSessionService::Initialize(UDBA_GameBackendClientSubsystem* InSubsystem, FDBA_GameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UDBA_GameBackendSessionService::GetSession(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Session service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Get(FString::Printf(TEXT("/api/sessions/%s"), *SessionId), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendSessionService::GetConnection(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Session service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Get(FString::Printf(TEXT("/api/sessions/%s/connection"), *SessionId), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendSessionService::RequestReconnectToken(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Session service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Post(FString::Printf(TEXT("/api/sessions/%s/reconnect-token"), *SessionId), TEXT("{}"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendSessionService::ConnectToDedicatedServer(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Session service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Get(FString::Printf(TEXT("/api/sessions/%s/connection"), *SessionId), [this, SessionId, Callback](const FDBA_GameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.IsSuccessful();
		if (!bSuccess)
		{
			Callback.ExecuteIfBound(false, Result.Message.IsEmpty() ? TEXT("Failed to get session connection.") : Result.Message, TEXT("{}"));
			return;
		}

		ConnectToDedicatedServer(SessionId, Result.DataJson, Callback);
	});
}

void UDBA_GameBackendSessionService::ConnectToDedicatedServer(const FString& SessionId, const FString& ConnectionDataJson, const FDBA_GameBackendResponseDelegate& Callback)
{
	FDBA_GameBackendSessionConnection Connection;
	if (!ParseConnectionData(ConnectionDataJson, Connection))
	{
		Callback.ExecuteIfBound(false, TEXT("Invalid connection data."), TEXT("{}"));
		return;
	}

	FString TravelUrl = BuildTravelUrl(Connection.Ip, Connection.Port, SessionId.IsEmpty() ? Connection.SessionId : SessionId, Connection.PlayerSessionToken);
	if (Subsystem.IsValid())
	{
		AppendTravelOption(TravelUrl, TEXT("PlayerId"), Connection.PlayerId.IsEmpty() ? Subsystem->GetPlayerId() : Connection.PlayerId);
	}
	if (TravelUrl.IsEmpty())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to build travel url."), TEXT("{}"));
		return;
	}

	if (!Subsystem.IsValid() || !Subsystem->GetGameInstance())
	{
		Callback.ExecuteIfBound(false, TEXT("Game instance unavailable."), TEXT("{}"));
		return;
	}

	UWorld* World = Subsystem->GetGameInstance()->GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		Callback.ExecuteIfBound(false, TEXT("PlayerController unavailable."), TEXT("{}"));
		return;
	}

	PC->ClientTravel(TravelUrl, TRAVEL_Absolute);
	Callback.ExecuteIfBound(true, FString(), TravelUrl);
}

FString UDBA_GameBackendSessionService::BuildTravelUrl(const FString& Ip, int32 Port, const FString& SessionId, const FString& PlayerSessionToken)
{
	const FString TrimmedIp = Ip.TrimStartAndEnd();
	if (TrimmedIp.IsEmpty() || Port <= 0)
	{
		return FString();
	}

	FString Url = FString::Printf(TEXT("%s:%d"), *TrimmedIp, Port);
	AppendTravelOption(Url, TEXT("SessionId"), SessionId);
	AppendTravelOption(Url, TEXT("PlayerSessionToken"), PlayerSessionToken);
	return Url;
}

bool UDBA_GameBackendSessionService::ParseConnectionData(const FString& DataJson, FDBA_GameBackendSessionConnection& OutConnection) const
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DataJson);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return false;
	}

	Root->TryGetStringField(TEXT("ip"), OutConnection.Ip);
	Root->TryGetNumberField(TEXT("port"), OutConnection.Port);
	Root->TryGetStringField(TEXT("sessionId"), OutConnection.SessionId);
	Root->TryGetStringField(TEXT("playerSessionToken"), OutConnection.PlayerSessionToken);
	Root->TryGetStringField(TEXT("playerId"), OutConnection.PlayerId);

	if (OutConnection.Ip.IsEmpty())
	{
		Root->TryGetStringField(TEXT("serverIp"), OutConnection.Ip);
	}
	if (OutConnection.Port <= 0)
	{
		Root->TryGetNumberField(TEXT("serverPort"), OutConnection.Port);
	}
	if (OutConnection.PlayerSessionToken.IsEmpty())
	{
		Root->TryGetStringField(TEXT("sessionToken"), OutConnection.PlayerSessionToken);
	}

	if (OutConnection.Ip.IsEmpty() && Root->HasTypedField<EJson::Object>(TEXT("connection")))
	{
		const TSharedPtr<FJsonObject> ConnectionObj = Root->GetObjectField(TEXT("connection"));
		if (ConnectionObj.IsValid())
		{
			ConnectionObj->TryGetStringField(TEXT("ip"), OutConnection.Ip);
			ConnectionObj->TryGetNumberField(TEXT("port"), OutConnection.Port);
			ConnectionObj->TryGetStringField(TEXT("sessionId"), OutConnection.SessionId);
			ConnectionObj->TryGetStringField(TEXT("playerSessionToken"), OutConnection.PlayerSessionToken);
			ConnectionObj->TryGetStringField(TEXT("playerId"), OutConnection.PlayerId);
		}
	}

	return !OutConnection.Ip.IsEmpty() && OutConnection.Port > 0;
}
