// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameBackendTypes.h"
#include "GameBackendTelemetryService.generated.h"

class UGameBackendClientSubsystem;
class FGameBackendHttpClient;

UCLASS(BlueprintType)
class GAMEBACKENDCLIENT_API UGameBackendTelemetryService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient);
	void Shutdown();

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Telemetry")
	void TrackEvent(const FString& EventName, const TMap<FString, FString>& Properties);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Telemetry")
	void TrackBatch(const TArray<FGameBackendTelemetryEvent>& Events);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Telemetry")
	void Flush();

	UFUNCTION(BlueprintPure, Category = "GameBackend|Telemetry")
	int32 GetQueueSize() const { return EventQueue.Num(); }

private:
	void SetupFlushTimer();
	void StopFlushTimer();
	void FlushInternal(bool bForce);
	FString BuildBatchPayload(const TArray<FGameBackendTelemetryEvent>& Events) const;

private:
	TWeakObjectPtr<UGameBackendClientSubsystem> Subsystem;
	FGameBackendHttpClient* HttpClient = nullptr;
	FTimerHandle FlushTimerHandle;
	TArray<FGameBackendTelemetryEvent> EventQueue;
	bool bFlushing = false;
	float FlushIntervalSeconds = 10.0f;
	int32 MaxQueueSize = 1000;
};
