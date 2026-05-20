// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameBackendClientSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Game Backend Client"))
class GAMEBACKENDCLIENT_API UGameBackendClientSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGameBackendClientSettings();

	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Connection")
	FString BackendBaseUrl = TEXT("http://localhost:5000");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString ClientVersion = TEXT("0.1.0");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString BuildNumber = TEXT("100");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString ConfigVersion = TEXT("bootstrap_v1");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString Channel = TEXT("dev");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString Platform = TEXT("Windows");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString Region = TEXT("local");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "HTTP", meta = (ClampMin = "1.0"))
	float RequestTimeoutSeconds = 15.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Telemetry", meta = (ClampMin = "1.0"))
	float TelemetryFlushIntervalSeconds = 10.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Telemetry", meta = (ClampMin = "10"))
	int32 TelemetryMaxQueueSize = 1000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "HTTP", meta = (ClampMin = "0"))
	int32 HttpRetryCount = 1;
};
