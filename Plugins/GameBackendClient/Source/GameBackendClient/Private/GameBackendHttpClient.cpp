// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendHttpClient.h"

#include "GameBackendClientSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Policies/CondensedJsonPrintPolicy.h"

namespace
{
	FString NormalizePath(const FString& InPath)
	{
		FString Normalized = InPath.TrimStartAndEnd();
		while (Normalized.StartsWith(TEXT("/")))
		{
			Normalized.RightChopInline(1);
		}
		return Normalized;
	}

	FString JsonToString(const TSharedPtr<FJsonValue>& Value)
	{
		if (!Value.IsValid())
		{
			return TEXT("{}");
		}

		FString Out;
		auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Out);
		FJsonSerializer::Serialize(Value.ToSharedRef(), TEXT(""), Writer);
		return Out;
	}

	FString MakeBodyFromObject(const TSharedPtr<FJsonObject>& Obj)
	{
		if (!Obj.IsValid())
		{
			return TEXT("{}");
		}

		FString Out;
		auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Out);
		FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);
		return Out;
	}
}

FGameBackendHttpClient::FGameBackendHttpClient(TWeakObjectPtr<UGameBackendClientSubsystem> InSubsystem)
	: Subsystem(InSubsystem)
{
}

void FGameBackendHttpClient::Get(const FString& Path, const FGameBackendHttpCallback& Callback, bool bRequiresAuth)
{
	FPendingRequest Request;
	Request.Method = TEXT("GET");
	Request.Path = Path;
	Request.bRequiresAuth = bRequiresAuth;
	Request.Callback = Callback;
	Send(MoveTemp(Request));
}

void FGameBackendHttpClient::Post(const FString& Path, const FString& JsonBody, const FGameBackendHttpCallback& Callback, bool bRequiresAuth)
{
	FPendingRequest Request;
	Request.Method = TEXT("POST");
	Request.Path = Path;
	Request.JsonBody = JsonBody;
	Request.bRequiresAuth = bRequiresAuth;
	Request.Callback = Callback;
	Send(MoveTemp(Request));
}

void FGameBackendHttpClient::Put(const FString& Path, const FString& JsonBody, const FGameBackendHttpCallback& Callback, bool bRequiresAuth)
{
	FPendingRequest Request;
	Request.Method = TEXT("PUT");
	Request.Path = Path;
	Request.JsonBody = JsonBody;
	Request.bRequiresAuth = bRequiresAuth;
	Request.Callback = Callback;
	Send(MoveTemp(Request));
}

void FGameBackendHttpClient::Patch(const FString& Path, const FString& JsonBody, const FGameBackendHttpCallback& Callback, bool bRequiresAuth)
{
	FPendingRequest Request;
	Request.Method = TEXT("PATCH");
	Request.Path = Path;
	Request.JsonBody = JsonBody;
	Request.bRequiresAuth = bRequiresAuth;
	Request.Callback = Callback;
	Send(MoveTemp(Request));
}

void FGameBackendHttpClient::Delete(const FString& Path, const FGameBackendHttpCallback& Callback, bool bRequiresAuth)
{
	FPendingRequest Request;
	Request.Method = TEXT("DELETE");
	Request.Path = Path;
	Request.bRequiresAuth = bRequiresAuth;
	Request.Callback = Callback;
	Send(MoveTemp(Request));
}

void FGameBackendHttpClient::Send(FPendingRequest Request)
{
	if (!Subsystem.IsValid())
	{
		FGameBackendHttpResult Result;
		Result.bHttpRequestOk = false;
		Result.Message = TEXT("Backend subsystem invalid.");
		Finish(Request, Result);
		return;
	}

	Request.TraceId = Request.TraceId.IsEmpty() ? BuildTraceId() : Request.TraceId;
	Request.StartTime = FPlatformTime::Seconds();

	const FString Url = BuildAbsoluteUrl(Request.Path);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(Request.Method);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetContentAsString(Request.JsonBody);
	ApplyHeaders(Request, HttpRequest);
	HttpRequest->SetTimeout(Subsystem->GetRequestTimeoutSeconds());

	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FGameBackendHttpClient::OnComplete, Request);
	if (!HttpRequest->ProcessRequest())
	{
		FGameBackendHttpResult Result;
		Result.bHttpRequestOk = false;
		Result.TraceId = Request.TraceId;
		Result.Message = TEXT("ProcessRequest failed to start.");
		Finish(Request, Result);
	}
}

void FGameBackendHttpClient::OnComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FPendingRequest Request)
{
	FGameBackendHttpResult Result;
	Result.bHttpRequestOk = bSucceeded && HttpResponse.IsValid();
	Result.TraceId = Request.TraceId;
	Result.DurationMs = (FPlatformTime::Seconds() - Request.StartTime) * 1000.0;
	Result.HttpStatus = HttpResponse.IsValid() ? HttpResponse->GetResponseCode() : 0;
	Result.RawBody = HttpResponse.IsValid() ? HttpResponse->GetContentAsString() : FString();

	if (HttpResponse.IsValid())
	{
		ParseEnvelope(Result.RawBody, Result);
	}
	else
	{
		Result.Message = TEXT("HTTP \u65e0\u54cd\u5e94\u3002");
	}

	UE_LOG(
		LogGameBackendClient,
		Log,
		TEXT("HTTP 璇锋眰 method=%s url=%s traceId=%s status=%d code=%s durationMs=%.2f"),
		*Request.Method,
		*BuildAbsoluteUrl(Request.Path),
		*Request.TraceId,
		Result.HttpStatus,
		*Result.Code,
		Result.DurationMs);

	if (Result.HttpStatus == 401 && Request.bRequiresAuth && Request.bAllowRefresh && Subsystem.IsValid())
	{
		Subsystem->RequestRefreshToken([this, Request](bool bRefreshOk)
		{
			if (!Subsystem.IsValid() || !bRefreshOk)
			{
				FGameBackendHttpResult FailResult;
				FailResult.bHttpRequestOk = false;
				FailResult.HttpStatus = 401;
				FailResult.Message = TEXT("\u9274\u6743\u5931\u8d25\u4e14\u5237\u65b0\u4ee4\u724c\u5931\u8d25\u3002");
				FailResult.TraceId = Request.TraceId;
				Finish(Request, FailResult);
				return;
			}

			FPendingRequest RetryRequest = Request;
			RetryRequest.bAllowRefresh = false;
			Send(MoveTemp(RetryRequest));
		});
		return;
	}

	if (ShouldRetry(Request, Result))
	{
		Retry(Request);
		return;
	}

	Finish(Request, Result);
}

FString FGameBackendHttpClient::BuildAbsoluteUrl(const FString& Path) const
{
	if (!Subsystem.IsValid())
	{
		return Path;
	}

	FString Base = Subsystem->GetBackendBaseUrl().TrimStartAndEnd();
	while (Base.EndsWith(TEXT("/")))
	{
		Base.LeftChopInline(1);
	}
	if (Path.StartsWith(TEXT("http://")) || Path.StartsWith(TEXT("https://")))
	{
		return Path;
	}

	const FString NormalizedPath = NormalizePath(Path);
	return NormalizedPath.IsEmpty() ? Base : Base + TEXT("/") + NormalizedPath;
}

FString FGameBackendHttpClient::BuildTraceId() const
{
	return FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
}

void FGameBackendHttpClient::ApplyHeaders(const FPendingRequest& Request, const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& HttpRequest)
{
	if (!Subsystem.IsValid())
	{
		return;
	}

	HttpRequest->SetHeader(TEXT("X-Client-Version"), Subsystem->GetClientVersion());
	HttpRequest->SetHeader(TEXT("X-Platform"), Subsystem->GetPlatformName());
	HttpRequest->SetHeader(TEXT("X-Trace-Id"), Request.TraceId);

	if (Request.bRequiresAuth)
	{
		const FString Token = Subsystem->GetAccessToken();
		if (!Token.IsEmpty())
		{
			HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Token));
		}
	}
}

bool FGameBackendHttpClient::ParseEnvelope(const FString& Body, FGameBackendHttpResult& OutResult) const
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutResult.Message = Body.IsEmpty() ? TEXT("Empty response body.") : TEXT("Invalid JSON response.");
		return false;
	}

	Root->TryGetStringField(TEXT("code"), OutResult.Code);
	Root->TryGetStringField(TEXT("message"), OutResult.Message);
	if (Root->HasField(TEXT("data")))
	{
		OutResult.DataJson = JsonToString(Root->TryGetField(TEXT("data")));
	}
	else
	{
		OutResult.DataJson = TEXT("{}");
	}

	if (OutResult.Code.IsEmpty())
	{
		OutResult.Code = TEXT("UNKNOWN");
	}

	return true;
}

void FGameBackendHttpClient::Finish(const FPendingRequest& Request, const FGameBackendHttpResult& Result) const
{
	if (Request.Callback)
	{
		Request.Callback(Result);
	}
}

bool FGameBackendHttpClient::ShouldRetry(const FPendingRequest& Request, const FGameBackendHttpResult& Result) const
{
	if (!Subsystem.IsValid())
	{
		return false;
	}

	const int32 MaxRetry = FMath::Max(0, Subsystem->GetHttpRetryCount());
	if (Request.Attempt >= MaxRetry)
	{
		return false;
	}

	if (!Result.bHttpRequestOk)
	{
		return true;
	}

	return Result.HttpStatus >= 500 && Result.HttpStatus <= 599;
}

void FGameBackendHttpClient::Retry(const FPendingRequest& Request) const
{
	FPendingRequest RetryRequest = Request;
	RetryRequest.Attempt++;
	UE_LOG(
		LogGameBackendClient,
		Warning,
		TEXT("HTTP 閲嶈瘯 method=%s path=%s attempt=%d traceId=%s"),
		*RetryRequest.Method,
		*RetryRequest.Path,
		RetryRequest.Attempt,
		*RetryRequest.TraceId);

	const_cast<FGameBackendHttpClient*>(this)->Send(MoveTemp(RetryRequest));
}
