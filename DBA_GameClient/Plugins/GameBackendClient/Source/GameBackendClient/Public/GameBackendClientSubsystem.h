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
#include "Subsystems/GameInstanceSubsystem.h"
#include "DBA_GameBackendTypes.h"
#include "DBA_GameBackendClientSubsystem.generated.h"

class FDBA_GameBackendHttpClient;
class UDBA_GameBackendAuthService;
class UDBA_GameBackendPlayerService;
class UDBA_GameBackendConfigService;
class UDBA_GameBackendRoomService;
class UDBA_GameBackendMatchService;
class UDBA_GameBackendSessionService;
class UDBA_GameBackendMailService;
class UDBA_GameBackendSupportService;
class UDBA_GameBackendTelemetryService;
class UDBA_GameBackendCrashService;

UCLASS(BlueprintType)
class GAMEBACKENDCLIENT_API UDBA_GameBackendClientSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Auth")
	bool IsLoggedIn() const;

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Auth")
	void Logout();

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Config")
	const FString& GetBackendBaseUrl() const { return BackendBaseUrl; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Config")
	const FString& GetClientVersion() const { return ClientVersion; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Config")
	const FString& GetPlatformName() const { return Platform; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Auth")
	FString GetAccessToken() const;

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Auth")
	FString GetRefreshToken() const;

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Auth")
	FString GetPlayerId() const;

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Auth")
	void SetAuthTokens(const FString& InAccessToken, const FString& InRefreshToken, const FString& InPlayerId);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Auth")
	void ClearAuthTokens();

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Service")
	UDBA_GameBackendAuthService* GetAuthService() const { return AuthService; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Service")
	UDBA_GameBackendPlayerService* GetPlayerService() const { return PlayerService; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Service")
	UDBA_GameBackendConfigService* GetConfigService() const { return ConfigService; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Service")
	UDBA_GameBackendRoomService* GetRoomService() const { return RoomService; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Service")
	UDBA_GameBackendMatchService* GetMatchService() const { return MatchService; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Service")
	UDBA_GameBackendSessionService* GetSessionService() const { return SessionService; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Service")
	UDBA_GameBackendMailService* GetMailService() const { return MailService; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Service")
	UDBA_GameBackendSupportService* GetSupportService() const { return SupportService; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Service")
	UDBA_GameBackendTelemetryService* GetTelemetryService() const { return TelemetryService; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Service")
	UDBA_GameBackendCrashService* GetCrashService() const { return CrashService; }

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Debug")
	void TestVersionCheck(const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Debug")
	void TestDevLogin(const FString& DisplayName, const FDBA_GameBackendAuthResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Debug")
	void TestGetProfile(const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Debug")
	void TestGetConfigBundle(const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Debug")
	void TestCreateRoom(const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Debug")
	void TestMatchmaking(const FString& Mode, const FString& RegionCode, const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Debug")
	void TestTelemetry();

	FDBA_GameBackendHttpClient* GetHttpClient() const;
	float GetRequestTimeoutSeconds() const;
	int32 GetHttpRetryCount() const;

	void QueueRefreshCallback(TFunction<void(bool)> Completion);
	void RequestRefreshToken(TFunction<void(bool)> Completion);

private:
	void InitializeServices();
	void ReleaseServices();
	void NotifyRefreshCompleted(bool bSuccess);

private:
	UPROPERTY(Transient)
	TObjectPtr<UDBA_GameBackendAuthService> AuthService;

	UPROPERTY(Transient)
	TObjectPtr<UDBA_GameBackendPlayerService> PlayerService;

	UPROPERTY(Transient)
	TObjectPtr<UDBA_GameBackendConfigService> ConfigService;

	UPROPERTY(Transient)
	TObjectPtr<UDBA_GameBackendRoomService> RoomService;

	UPROPERTY(Transient)
	TObjectPtr<UDBA_GameBackendMatchService> MatchService;

	UPROPERTY(Transient)
	TObjectPtr<UDBA_GameBackendSessionService> SessionService;

	UPROPERTY(Transient)
	TObjectPtr<UDBA_GameBackendMailService> MailService;

	UPROPERTY(Transient)
	TObjectPtr<UDBA_GameBackendSupportService> SupportService;

	UPROPERTY(Transient)
	TObjectPtr<UDBA_GameBackendTelemetryService> TelemetryService;

	UPROPERTY(Transient)
	TObjectPtr<UDBA_GameBackendCrashService> CrashService;

	TSharedPtr<FDBA_GameBackendHttpClient> HttpClient;

	FString BackendBaseUrl;
	FString ClientVersion;
	FString BuildNumber;
	FString Channel;
	FString Platform;
	FString Region;
	float RequestTimeoutSeconds = 15.0f;
	int32 HttpRetryCount = 1;

	FString AccessToken;
	FString RefreshToken;
	FString PlayerId;

	bool bRefreshingToken = false;
	TArray<TFunction<void(bool)>> RefreshCallbacks;
};
