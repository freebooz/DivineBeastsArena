// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendMatchService.h"

#include "GameBackendHttpClient.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	void ExecuteResponse(const FGameBackendResponseDelegate& Callback, const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("Request failed.") : Result.Message);
		Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
	}
}

void UGameBackendMatchService::Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UGameBackendMatchService::CreateTicket(const FString& Mode, const FString& RegionCode, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Match service unavailable."), TEXT("{}"));
		return;
	}

	const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("mode"), Mode);
	Json->SetStringField(TEXT("region"), RegionCode);

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Json, Writer);

	HttpClient->Post(TEXT("/api/matchmaking/tickets"), Body, [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendMatchService::GetTicket(const FString& TicketId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Match service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(FString::Printf(TEXT("/api/matchmaking/tickets/%s"), *TicketId), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendMatchService::CancelTicket(const FString& TicketId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Match service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Delete(FString::Printf(TEXT("/api/matchmaking/tickets/%s"), *TicketId), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

