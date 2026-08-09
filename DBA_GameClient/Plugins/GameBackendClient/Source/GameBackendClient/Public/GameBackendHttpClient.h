// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient / GameBackendClient Unreal 插件。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameBackendTypes.h"
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

/**
 * 单个 HTTP 请求的传输选项。前台 ApiClient 使用该结构将关联标识、刷新策略和超时
 * 明确交给唯一传输层；旧服务仍可继续使用 Get/Post 等兼容入口。
 */
struct FDBA_GameBackendHttpRequestOptions
{
	bool bRequiresAuth = true;
	bool bAllowRefresh = true;
	FString CorrelationId;
	float TimeoutSeconds = 0.0f;
	/** 由领域服务声明的协议头；不得放入认证凭据或玩家密码。 */
	TMap<FString, FString> AdditionalHeaders;
};

class GAMEBACKENDCLIENT_API FDBA_GameBackendHttpClient
{
public:
	explicit FDBA_GameBackendHttpClient(TWeakObjectPtr<UDBA_GameBackendClientSubsystem> InSubsystem);

	void Get(const FString& Path, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Post(const FString& Path, const FString& JsonBody, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Put(const FString& Path, const FString& JsonBody, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Patch(const FString& Path, const FString& JsonBody, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth = true);
	void Delete(const FString& Path, const FDBA_GameBackendHttpCallback& Callback, bool bRequiresAuth = true);

	/** 唯一可取消传输入口。返回的句柄可在刷新等待、重试或实际网络请求期间取消。 */
	FGuid SendRequest(
		const FString& Method,
		const FString& Path,
		const FString& JsonBody,
		const FDBA_GameBackendHttpRequestOptions& Options,
		const FDBA_GameBackendHttpCallback& Callback);

	bool CancelRequest(const FGuid& RequestId);
	void CancelAllRequests();

private:
	struct FPendingRequest
	{
		FString Method;
		FString Path;
		FString JsonBody;
		bool bRequiresAuth = true;
		int32 Attempt = 0;
		bool bAllowRefresh = true;
		float TimeoutSeconds = 0.0f;
		double StartTime = 0.0;
		FString TraceId;
		TMap<FString, FString> AdditionalHeaders;
		FGuid RequestId;
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
	bool IsCancelled(const FGuid& RequestId) const;
	void ForgetRequest(const FGuid& RequestId);

private:
	TWeakObjectPtr<UDBA_GameBackendClientSubsystem> Subsystem;
	TMap<FGuid, FHttpRequestPtr> ActiveRequests;
	TSet<FGuid> CancelledRequests;
};
