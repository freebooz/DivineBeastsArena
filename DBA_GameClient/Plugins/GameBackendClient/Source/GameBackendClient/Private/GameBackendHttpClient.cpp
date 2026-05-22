// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "DBA_GameBackendHttpClient.h"

#include "DBA_GameBackendClientSubsystem.h"
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

FDBA_GameBackendHttpClient::FDBA_GameBackendHttpClient(TWeakObjectPtr<UDBA_GameBackendClientSubsystem> InSubsystem)
	: Subsystem(InSubsystem)
{
}

void FDBA_GameBackendHttpClient::Get(const FString& Path, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth)
{
	FPendingRequest Request;
	Request.Method = TEXT("GET");
	Request.Path = Path;
	Request.bRequiresAuth = bRequiresAuth;
	Request.Callback = Callback;
	Send(MoveTemp(Request));
}

void FDBA_GameBackendHttpClient::Post(const FString& Path, const FString& JsonBody, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth)
{
	FPendingRequest Request;
	Request.Method = TEXT("POST");
	Request.Path = Path;
	Request.JsonBody = JsonBody;
	Request.bRequiresAuth = bRequiresAuth;
	Request.Callback = Callback;
	Send(MoveTemp(Request));
}

void FDBA_GameBackendHttpClient::Put(const FString& Path, const FString& JsonBody, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth)
{
	FPendingRequest Request;
	Request.Method = TEXT("PUT");
	Request.Path = Path;
	Request.JsonBody = JsonBody;
	Request.bRequiresAuth = bRequiresAuth;
	Request.Callback = Callback;
	Send(MoveTemp(Request));
}

void FDBA_GameBackendHttpClient::Patch(const FString& Path, const FString& JsonBody, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth)
{
	FPendingRequest Request;
	Request.Method = TEXT("PATCH");
	Request.Path = Path;
	Request.JsonBody = JsonBody;
	Request.bRequiresAuth = bRequiresAuth;
	Request.Callback = Callback;
	Send(MoveTemp(Request));
}

void FDBA_GameBackendHttpClient::Delete(const FString& Path, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth)
{
	FPendingRequest Request;
	Request.Method = TEXT("DELETE");
	Request.Path = Path;
	Request.bRequiresAuth = bRequiresAuth;
	Request.Callback = Callback;
	Send(MoveTemp(Request));
}

void FDBA_GameBackendHttpClient::Send(FPendingRequest Request)
{
	if (!Subsystem.IsValid())
	{
		FDBA_GameBackendHttpResult Result;
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

	HttpRequest->OnProcessRequestComplete().BindRaw(this, &FDBA_GameBackendHttpClient::OnComplete, Request);
	if (!HttpRequest->ProcessRequest())
	{
		FDBA_GameBackendHttpResult Result;
		Result.bHttpRequestOk = false;
		Result.TraceId = Request.TraceId;
		Result.Message = TEXT("HTTP 请求启动失败。");
		Finish(Request, Result);
	}
}

void FDBA_GameBackendHttpClient::OnComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FPendingRequest Request)
{
	FDBA_GameBackendHttpResult Result;
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
		LogDBA_GameBackendClient,
		Log,
		TEXT("HTTP 请求：方法=%s 地址=%s 追踪ID=%s 状态码=%d 业务码=%s 耗时毫秒=%.2f"),
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
				FDBA_GameBackendHttpResult FailResult;
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

FString FDBA_GameBackendHttpClient::BuildAbsoluteUrl(const FString& Path) const
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

FString FDBA_GameBackendHttpClient::BuildTraceId() const
{
	return FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
}

void FDBA_GameBackendHttpClient::ApplyHeaders(const FPendingRequest& Request, const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& HttpRequest)
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

bool FDBA_GameBackendHttpClient::ParseEnvelope(const FString& Body, FDBA_GameBackendHttpResult& OutResult) const
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutResult.Message = Body.IsEmpty() ? TEXT("响应体为空。") : TEXT("JSON 响应格式无效。");
		return false;
	}

	Root->TryGetStringField(TEXT("code"), OutResult.Code);
	Root->TryGetStringField(TEXT("message"), OutResult.Message);
	if (Root->HasTypedField<EJson::Boolean>(TEXT("success")))
	{
		OutResult.bApiSuccess = Root->GetBoolField(TEXT("success"));
	}
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
		OutResult.Code = OutResult.bApiSuccess ? TEXT("OK") : TEXT("ERROR");
	}

	return true;
}

void FDBA_GameBackendHttpClient::Finish(const FPendingRequest& Request, const FDBA_GameBackendHttpResult& Result) const
{
	if (Request.Callback)
	{
		Request.Callback(Result);
	}
}

bool FDBA_GameBackendHttpClient::ShouldRetry(const FPendingRequest& Request, const FDBA_GameBackendHttpResult& Result) const
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

void FDBA_GameBackendHttpClient::Retry(const FPendingRequest& Request) const
{
	FPendingRequest RetryRequest = Request;
	RetryRequest.Attempt++;
	UE_LOG(
		LogDBA_GameBackendClient,
		Warning,
		TEXT("HTTP 重试：方法=%s 路径=%s 次数=%d 追踪ID=%s"),
		*RetryRequest.Method,
		*RetryRequest.Path,
		RetryRequest.Attempt,
		*RetryRequest.TraceId);

	const_cast<FDBA_GameBackendHttpClient*>(this)->Send(MoveTemp(RetryRequest));
}
