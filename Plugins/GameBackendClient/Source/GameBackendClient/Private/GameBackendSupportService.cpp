// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendSupportService.h"

#include "GameBackendHttpClient.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	void ExecuteResponse(const FGameBackendResponseDelegate& Callback, const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("请求失败。") : Result.Message);
		Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
	}

	FString BuildPayload(const TFunction<void(TSharedRef<FJsonObject>)>& Fill)
	{
		const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
		Fill(Json);
		FString Body;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
		FJsonSerializer::Serialize(Json, Writer);
		return Body;
	}
}

void UGameBackendSupportService::Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UGameBackendSupportService::SubmitTicket(const FGameBackendSupportTicketRequest& Request, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("客服服务不可用。"), TEXT("{}"));
		return;
	}

	const FString Body = BuildPayload([&Request](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("subject"), Request.Subject);
		Json->SetStringField(TEXT("category"), Request.Category);
		Json->SetStringField(TEXT("content"), Request.Content);
	});

	HttpClient->Post(TEXT("/api/support/tickets"), Body, [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendSupportService::GetMyTickets(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("客服服务不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/support/tickets/me"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendSupportService::SubmitReport(const FGameBackendReportRequest& Request, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("举报服务不可用。"), TEXT("{}"));
		return;
	}

	const FString Body = BuildPayload([&Request](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("targetPlayerId"), Request.TargetPlayerId);
		Json->SetStringField(TEXT("reasonCode"), Request.ReasonCode);
		Json->SetStringField(TEXT("description"), Request.Description);
	});

	HttpClient->Post(TEXT("/api/reports"), Body, [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendSupportService::GetMyReports(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("举报服务不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/reports/me"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendSupportService::SubmitAppeal(const FString& AppealJson, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("申诉服务不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Post(TEXT("/api/appeals"), AppealJson.IsEmpty() ? TEXT("{}") : AppealJson, [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

