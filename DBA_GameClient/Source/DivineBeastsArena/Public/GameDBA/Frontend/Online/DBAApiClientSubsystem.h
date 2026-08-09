// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameDBA/Core/DBAResultTypes.h"
#include "DBAApiClientSubsystem.generated.h"

class UDBA_GameBackendClientSubsystem;
class UDBASecureTokenStorage;

enum class EDBAApiHttpVerb : uint8
{
	Get,
	Post,
	Put,
	Delete
};

/** 由 Domain Service 组装的请求；禁止从 Widget 直接调用或拼接 JSON。 */
struct FDBAApiRequest
{
	EDBAApiHttpVerb Verb = EDBAApiHttpVerb::Get;
	FString Path;
	FString JsonBody;
	bool bRequiresAuthentication = true;
	bool bRetryAfterAuthenticationRefresh = true;
	float TimeoutSeconds = 0.0f;
	FString CorrelationId;
	/** 仅用于领域 API 约定的附加请求头，例如幂等键与删除确认。 */
	TMap<FString, FString> Headers;
};

/** 仅交给 Domain Service 的响应。UI 只消费 Result.ApiError，不消费 DomainJson。 */
struct FDBAApiResponse
{
	FDBAOperationResult Result;
	FString DomainJson;
	FString CorrelationId;
	int32 HttpStatusCode = 0;
};

using FDBAApiCompletion = TFunction<void(const FDBAApiResponse&)>;
using FDBAApiMockTransport = TFunction<void(const FDBAApiRequest&, TFunction<void(const FDBAApiResponse&)>)>;
using FDBAApiMockRefresh = TFunction<void(TFunction<void(bool)>)>;

/**
 * 刷新令牌单飞请求最终失败时只广播一次。
 * Flow 订阅该事件并统一执行登出清理；各个 Widget/领域子系统不得自行处理 Token 过期。
 */
DECLARE_MULTICAST_DELEGATE(FDBAOnAuthenticationRefreshFailed);

/**
 * 前台唯一 HTTP/JSON 入口。它不创建第二套传输，而是委托 GameBackendClient 的
 * FDBA_GameBackendHttpClient，并集中处理请求生命周期、注销失效、401 刷新和错误映射。
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAApiClientSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool IsSupportedInCurrentEnvironment() const override;

	FGuid Send(const FDBAApiRequest& Request, UObject* CallbackOwner, FDBAApiCompletion Completion);
	FGuid Get(const FString& Path, bool bRequiresAuthentication, UObject* CallbackOwner, FDBAApiCompletion Completion);
	FGuid Post(const FString& Path, const FString& JsonBody, bool bRequiresAuthentication, UObject* CallbackOwner, FDBAApiCompletion Completion);
	FGuid Put(const FString& Path, const FString& JsonBody, bool bRequiresAuthentication, UObject* CallbackOwner, FDBAApiCompletion Completion);
	FGuid Delete(const FString& Path, bool bRequiresAuthentication, UObject* CallbackOwner, FDBAApiCompletion Completion);

	bool CancelRequest(const FGuid& RequestId);
	void CancelRequestsFor(const UObject* CallbackOwner);
	void CancelOutstandingRequests();

	void SetAuthenticationTokens(const FString& AccessToken, const FString& RefreshToken, const FString& PlayerId);
	void SetRefreshTokenPersistenceEnabled(bool bEnabled);
	bool LoadDevelopmentRefreshToken(FString& OutRefreshToken) const;
	void InvalidateSession();

	/**
	 * 返回认证刷新失败事件。该事件仅表示 ApiClient 已尝试单飞 Refresh 且失败，
	 * 不携带 Token、RefreshToken 或远端响应正文。
	 */
	FDBAOnAuthenticationRefreshFailed& OnAuthenticationRefreshFailed() { return AuthenticationRefreshFailed; }

	/** 只供自动化契约或无后端前端调试注入；不得在 Widget 中设置。 */
	void SetMockTransportForTests(FDBAApiMockTransport InMockTransport);
	void ClearMockTransportForTests();
	void SetMockRefreshForTests(FDBAApiMockRefresh InMockRefresh);

private:
	struct FPendingRequest
	{
		FDBAApiRequest Request;
		TWeakObjectPtr<UObject> CallbackOwner;
		FDBAApiCompletion Completion;
		FGuid TransportRequestId;
		uint64 SessionGeneration = 0;
		bool bRefreshAttempted = false;
	};

	void DispatchTransport(const FGuid& RequestId);
	void HandleTransportResponse(const FGuid& RequestId, const FDBAApiResponse& Response);
	void QueueAuthenticationRefresh(const FGuid& RequestId);
	void HandleAuthenticationRefreshCompleted(bool bSuccess);
	void CompleteRequest(const FGuid& RequestId, const FDBAApiResponse& Response);
	FDBAApiResponse MakeResponseFromTransportResult(const struct FDBA_GameBackendHttpResult& TransportResult) const;
	UDBA_GameBackendClientSubsystem* GetBackendClient() const;
	static FString ToHttpVerb(EDBAApiHttpVerb Verb);

private:
	UPROPERTY(Transient)
	TObjectPtr<UDBASecureTokenStorage> TokenStorage;

	TMap<FGuid, FPendingRequest> PendingRequests;
	FDBAApiMockTransport MockTransport;
	FDBAApiMockRefresh MockRefresh;
	uint64 SessionGeneration = 0;
	bool bRefreshInFlight = false;
	bool bPersistRefreshToken = true;
	FDBAOnAuthenticationRefreshFailed AuthenticationRefreshFailed;
};
