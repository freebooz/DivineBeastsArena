// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameBackendTypes.h"
#include "GameBackendConfigService.generated.h"

class UGameBackendClientSubsystem;
class FGameBackendHttpClient;

UCLASS(BlueprintType)
class GAMEBACKENDCLIENT_API UGameBackendConfigService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Config")
	void VersionCheck(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Config")
	void GetMaintenanceStatus(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Config")
	void GetAnnouncementsPopup(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Config")
	void GetConfigManifest(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Config")
	void GetConfigBundle(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Config")
	void GetConfigByKey(const FString& ConfigKey, const FGameBackendResponseDelegate& Callback);

private:
	TWeakObjectPtr<UGameBackendClientSubsystem> Subsystem;
	FGameBackendHttpClient* HttpClient = nullptr;
};
