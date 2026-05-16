// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendConfigService.h"

#include "GameBackendClientSubsystem.h"
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

void UGameBackendConfigService::Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UGameBackendConfigService::VersionCheck(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient || !Subsystem.IsValid())
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}

	const TSharedRef<FJsonObject> Request = MakeShared<FJsonObject>();
	Request->SetStringField(TEXT("clientVersion"), Subsystem->GetClientVersion());
	Request->SetStringField(TEXT("platform"), Subsystem->GetPlatformName());

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Request, Writer);

	HttpClient->Post(TEXT("/api/client/version-check"), Body, [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); }, false);
}

void UGameBackendConfigService::GetMaintenanceStatus(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/maintenance/status"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); }, false);
}

void UGameBackendConfigService::GetAnnouncementsPopup(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/announcements/popup"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); }, false);
}

void UGameBackendConfigService::GetConfigManifest(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/config/manifest"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendConfigService::GetConfigBundle(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/config/bundle"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendConfigService::GetConfigByKey(const FString& ConfigKey, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}
	const FString SafeKey = ConfigKey.TrimStartAndEnd();
	HttpClient->Get(FString::Printf(TEXT("/api/config/%s"), *SafeKey), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

