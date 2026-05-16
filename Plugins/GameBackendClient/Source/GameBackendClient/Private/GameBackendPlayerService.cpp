// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendPlayerService.h"

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

void UGameBackendPlayerService::Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UGameBackendPlayerService::GetMyProfile(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/profile"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendPlayerService::UpdateMyProfile(const FString& ProfileJson, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Patch(TEXT("/api/players/me/profile"), ProfileJson.IsEmpty() ? TEXT("{}") : ProfileJson, [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendPlayerService::GetMySettings(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/settings"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendPlayerService::UpdateMySettings(const FString& SettingsJson, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Put(TEXT("/api/players/me/settings"), SettingsJson.IsEmpty() ? TEXT("{}") : SettingsJson, [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendPlayerService::GetMyStats(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/stats"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendPlayerService::GetMyUnlocks(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/unlocks"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendPlayerService::GetMyInventory(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/inventory"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendPlayerService::GetMyMatches(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP client unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/matches"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}
