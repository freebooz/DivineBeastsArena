// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameBackendTypes.h"
#include "GameBackendAuthService.generated.h"

class UGameBackendClientSubsystem;
class FGameBackendHttpClient;

UCLASS(BlueprintType)
class GAMEBACKENDCLIENT_API UGameBackendAuthService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Auth")
	void DevLogin(const FString& DisplayName, const FGameBackendAuthResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Auth")
	void GuestLogin(const FGameBackendGuestLoginRequest& Request, const FGameBackendAuthResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Auth")
	void SteamLogin(const FString& SteamTicket, const FGameBackendAuthResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Auth")
	void EosLogin(const FString& EosToken, const FGameBackendAuthResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Auth")
	void RefreshToken(const FGameBackendAuthResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Auth")
	void Logout(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Auth")
	void GetMe(const FGameBackendResponseDelegate& Callback);

private:
	void HandleAuthResponse(const FGameBackendHttpResult& Result, const FGameBackendAuthResponseDelegate& Callback);
	static bool TryExtractTokens(const FString& DataJson, FGameBackendAuthTokens& OutTokens);

private:
	TWeakObjectPtr<UGameBackendClientSubsystem> Subsystem;
	FGameBackendHttpClient* HttpClient = nullptr;
};
