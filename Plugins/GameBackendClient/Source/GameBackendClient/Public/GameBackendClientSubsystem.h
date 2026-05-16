// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameBackendTypes.h"
#include "GameBackendClientSubsystem.generated.h"

class FGameBackendHttpClient;
class UGameBackendAuthService;
class UGameBackendPlayerService;
class UGameBackendConfigService;
class UGameBackendRoomService;
class UGameBackendMatchService;
class UGameBackendSessionService;
class UGameBackendMailService;
class UGameBackendSupportService;
class UGameBackendTelemetryService;
class UGameBackendCrashService;

UCLASS(BlueprintType)
class GAMEBACKENDCLIENT_API UGameBackendClientSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "GameBackend|Auth")
	bool IsLoggedIn() const;

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Auth")
	void Logout();

	UFUNCTION(BlueprintPure, Category = "GameBackend|Config")
	const FString& GetBackendBaseUrl() const { return BackendBaseUrl; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Config")
	const FString& GetClientVersion() const { return ClientVersion; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Config")
	const FString& GetPlatformName() const { return Platform; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Auth")
	FString GetAccessToken() const;

	UFUNCTION(BlueprintPure, Category = "GameBackend|Auth")
	FString GetRefreshToken() const;

	UFUNCTION(BlueprintPure, Category = "GameBackend|Auth")
	FString GetPlayerId() const;

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Auth")
	void SetAuthTokens(const FString& InAccessToken, const FString& InRefreshToken, const FString& InPlayerId);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Auth")
	void ClearAuthTokens();

	UFUNCTION(BlueprintPure, Category = "GameBackend|Service")
	UGameBackendAuthService* GetAuthService() const { return AuthService; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Service")
	UGameBackendPlayerService* GetPlayerService() const { return PlayerService; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Service")
	UGameBackendConfigService* GetConfigService() const { return ConfigService; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Service")
	UGameBackendRoomService* GetRoomService() const { return RoomService; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Service")
	UGameBackendMatchService* GetMatchService() const { return MatchService; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Service")
	UGameBackendSessionService* GetSessionService() const { return SessionService; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Service")
	UGameBackendMailService* GetMailService() const { return MailService; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Service")
	UGameBackendSupportService* GetSupportService() const { return SupportService; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Service")
	UGameBackendTelemetryService* GetTelemetryService() const { return TelemetryService; }

	UFUNCTION(BlueprintPure, Category = "GameBackend|Service")
	UGameBackendCrashService* GetCrashService() const { return CrashService; }

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Debug")
	void TestVersionCheck(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Debug")
	void TestDevLogin(const FString& DisplayName, const FGameBackendAuthResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Debug")
	void TestGetProfile(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Debug")
	void TestGetConfigBundle(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Debug")
	void TestCreateRoom(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Debug")
	void TestMatchmaking(const FString& Mode, const FString& RegionCode, const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Debug")
	void TestTelemetry();

	FGameBackendHttpClient* GetHttpClient() const;
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
	TObjectPtr<UGameBackendAuthService> AuthService;

	UPROPERTY(Transient)
	TObjectPtr<UGameBackendPlayerService> PlayerService;

	UPROPERTY(Transient)
	TObjectPtr<UGameBackendConfigService> ConfigService;

	UPROPERTY(Transient)
	TObjectPtr<UGameBackendRoomService> RoomService;

	UPROPERTY(Transient)
	TObjectPtr<UGameBackendMatchService> MatchService;

	UPROPERTY(Transient)
	TObjectPtr<UGameBackendSessionService> SessionService;

	UPROPERTY(Transient)
	TObjectPtr<UGameBackendMailService> MailService;

	UPROPERTY(Transient)
	TObjectPtr<UGameBackendSupportService> SupportService;

	UPROPERTY(Transient)
	TObjectPtr<UGameBackendTelemetryService> TelemetryService;

	UPROPERTY(Transient)
	TObjectPtr<UGameBackendCrashService> CrashService;

	TSharedPtr<FGameBackendHttpClient> HttpClient;

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
