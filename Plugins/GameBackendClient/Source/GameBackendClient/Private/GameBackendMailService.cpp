// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendMailService.h"

#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"

namespace
{
	void ExecuteResponse(const FGameBackendResponseDelegate& Callback, const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.IsSuccessful();
		const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("Request failed.") : Result.Message);
		Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
	}
}

void UGameBackendMailService::Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UGameBackendMailService::GetMyMails(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Mail service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/mails"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendMailService::GetMailDetail(const FString& MailId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Mail service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(FString::Printf(TEXT("/api/players/me/mails/%s"), *MailId), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendMailService::MarkRead(const FString& MailId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Mail service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Post(FString::Printf(TEXT("/api/players/me/mails/%s/read"), *MailId), TEXT("{}"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendMailService::ClaimMail(const FString& MailId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Mail service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Post(FString::Printf(TEXT("/api/players/me/mails/%s/claim"), *MailId), TEXT("{}"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendMailService::ClaimAll(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Mail service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Post(TEXT("/api/players/me/mails/claim-all"), TEXT("{}"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}
