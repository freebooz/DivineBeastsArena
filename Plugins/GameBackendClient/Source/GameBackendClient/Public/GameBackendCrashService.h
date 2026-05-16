// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameBackendTypes.h"
#include "GameBackendCrashService.generated.h"

class UGameBackendClientSubsystem;
class FGameBackendHttpClient;

UCLASS(BlueprintType)
class GAMEBACKENDCLIENT_API UGameBackendCrashService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Crash")
	void ScanCrashFiles();

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Crash")
	void UploadCrashReport(const FString& CrashFilePath, const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Crash")
	void UploadClientLog(const FString& LogFilePath, const FGameBackendResponseDelegate& Callback);

private:
	void UploadCrashDirectory(const FString& CrashDir);
	void TryUploadLatestClientLog();
	bool IsAlreadyUploaded(const FString& FilePath) const;
	void MarkUploaded(const FString& FilePath) const;
	FString BuildFileUploadPayload(const FString& FilePath, int64 MaxBytes, FString& OutError) const;

private:
	TWeakObjectPtr<UGameBackendClientSubsystem> Subsystem;
	FGameBackendHttpClient* HttpClient = nullptr;
	int64 MaxLogUploadBytes = 10 * 1024 * 1024;
};
