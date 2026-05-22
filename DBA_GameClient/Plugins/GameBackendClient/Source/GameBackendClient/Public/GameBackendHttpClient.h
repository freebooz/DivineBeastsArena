// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "DBA_GameBackendTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

class UDBA_GameBackendClientSubsystem;

struct FDBA_GameBackendHttpResult
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

using FDBA_GameBackendHttpCallback = TFunction<void(const FDBA_GameBackendHttpResult&)>;

class GAMEBACKENDCLIENT_API FDBA_GameBackendHttpClient
{
public:
	explicit FDBA_GameBackendHttpClient(TWeakObjectPtr<UDBA_GameBackendClientSubsystem> InSubsystem);

	void Get(const FString& Path, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Post(const FString& Path, const FString& JsonBody, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Put(const FString& Path, const FString& JsonBody, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Patch(const FString& Path, const FString& JsonBody, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Delete(const FString& Path, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth = true);

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
		FDBA_GameBackendHttpCallback Callback;
	};

	void Send(FPendingRequest Request);
	void OnComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FPendingRequest Request);
	FString BuildAbsoluteUrl(const FString& Path) const;
	FString BuildTraceId() const;
	void ApplyHeaders(const FPendingRequest& Request, const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& HttpRequest);
	bool ParseEnvelope(const FString& Body, FDBA_GameBackendHttpResult& OutResult) const;
	void Finish(const FPendingRequest& Request, const FDBA_GameBackendHttpResult& Result) const;
	bool ShouldRetry(const FPendingRequest& Request, const FDBA_GameBackendHttpResult& Result) const;
	void Retry(const FPendingRequest& Request) const;

private:
	TWeakObjectPtr<UDBA_GameBackendClientSubsystem> Subsystem;
};
