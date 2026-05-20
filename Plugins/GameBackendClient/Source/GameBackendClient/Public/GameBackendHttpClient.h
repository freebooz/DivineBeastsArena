// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameBackendTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

class UGameBackendClientSubsystem;

struct FGameBackendHttpResult
{
	bool bHttpRequestOk = false;
	bool bApiSuccess = true;
	int32 HttpStatus = 0;
	FString Code;
	FString Message;
	FString DataJson;
	FString TraceId;
	FString RawBody;
	double DurationMs = 0.0;

	bool IsSuccessful() const
	{
		return bHttpRequestOk && bApiSuccess && HttpStatus >= 200 && HttpStatus < 300;
	}
};

using FGameBackendHttpCallback = TFunction<void(const FGameBackendHttpResult&)>;

class GAMEBACKENDCLIENT_API FGameBackendHttpClient
{
public:
	explicit FGameBackendHttpClient(TWeakObjectPtr<UGameBackendClientSubsystem> InSubsystem);

	void Get(const FString& Path, const FGameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Post(const FString& Path, const FString& JsonBody, const FGameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Put(const FString& Path, const FString& JsonBody, const FGameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Patch(const FString& Path, const FString& JsonBody, const FGameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Delete(const FString& Path, const FGameBackendHttpCallback& Callback, bool bRequiresAuth = true);

private:
	struct FPendingRequest
	{
		FString Method;
		FString Path;
		FString JsonBody;
		bool bRequiresAuth = true;
		int32 Attempt = 0;
		bool bAllowRefresh = true;
		double StartTime = 0.0;
		FString TraceId;
		FGameBackendHttpCallback Callback;
	};

	void Send(FPendingRequest Request);
	void OnComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FPendingRequest Request);
	FString BuildAbsoluteUrl(const FString& Path) const;
	FString BuildTraceId() const;
	void ApplyHeaders(const FPendingRequest& Request, const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& HttpRequest);
	bool ParseEnvelope(const FString& Body, FGameBackendHttpResult& OutResult) const;
	void Finish(const FPendingRequest& Request, const FGameBackendHttpResult& Result) const;
	bool ShouldRetry(const FPendingRequest& Request, const FGameBackendHttpResult& Result) const;
	void Retry(const FPendingRequest& Request) const;

private:
	TWeakObjectPtr<UGameBackendClientSubsystem> Subsystem;
};
