// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendSessionService.h"

#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "GameFramework/PlayerController.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	void ExecuteResponse(const FGameBackendResponseDelegate& Callback, const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.IsSuccessful();
		const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("Request failed.") : Result.Message);
		Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
	}
}

void UGameBackendSessionService::Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UGameBackendSessionService::GetSession(const FString& SessionId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Session service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Get(FString::Printf(TEXT("/api/sessions/%s"), *SessionId), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendSessionService::GetConnection(const FString& SessionId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Session service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Get(FString::Printf(TEXT("/api/sessions/%s/connection"), *SessionId), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendSessionService::RequestReconnectToken(const FString& SessionId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Session service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Post(FString::Printf(TEXT("/api/sessions/%s/reconnect-token"), *SessionId), TEXT("{}"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendSessionService::ConnectToDedicatedServer(const FString& SessionId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Session service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Get(FString::Printf(TEXT("/api/sessions/%s/connection"), *SessionId), [this, SessionId, Callback](const FGameBackendHttpResult& Result)
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

void UGameBackendSessionService::ConnectToDedicatedServer(const FString& SessionId, const FString& ConnectionDataJson, const FGameBackendResponseDelegate& Callback)
{
	FGameBackendSessionConnection Connection;
	if (!ParseConnectionData(ConnectionDataJson, Connection))
	{
		Callback.ExecuteIfBound(false, TEXT("Invalid connection data."), TEXT("{}"));
		return;
	}

	const FString TravelUrl = BuildTravelUrl(Connection.Ip, Connection.Port, SessionId.IsEmpty() ? Connection.SessionId : SessionId, Connection.PlayerSessionToken);
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

FString UGameBackendSessionService::BuildTravelUrl(const FString& Ip, int32 Port, const FString& SessionId, const FString& PlayerSessionToken)
{
	const FString TrimmedIp = Ip.TrimStartAndEnd();
	if (TrimmedIp.IsEmpty() || Port <= 0)
	{
		return FString();
	}

	FString Url = FString::Printf(TEXT("%s:%d"), *TrimmedIp, Port);
	if (!SessionId.IsEmpty())
	{
		Url += FString::Printf(TEXT("?SessionId=%s"), *SessionId);
	}
	if (!PlayerSessionToken.IsEmpty())
	{
		Url += FString::Printf(TEXT("?PlayerSessionToken=%s"), *PlayerSessionToken);
	}
	return Url;
}

bool UGameBackendSessionService::ParseConnectionData(const FString& DataJson, FGameBackendSessionConnection& OutConnection) const
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

	if (OutConnection.Ip.IsEmpty() && Root->HasTypedField<EJson::Object>(TEXT("connection")))
	{
		const TSharedPtr<FJsonObject> ConnectionObj = Root->GetObjectField(TEXT("connection"));
		if (ConnectionObj.IsValid())
		{
			ConnectionObj->TryGetStringField(TEXT("ip"), OutConnection.Ip);
			ConnectionObj->TryGetNumberField(TEXT("port"), OutConnection.Port);
			ConnectionObj->TryGetStringField(TEXT("sessionId"), OutConnection.SessionId);
			ConnectionObj->TryGetStringField(TEXT("playerSessionToken"), OutConnection.PlayerSessionToken);
		}
	}

	return !OutConnection.Ip.IsEmpty() && OutConnection.Port > 0;
}
