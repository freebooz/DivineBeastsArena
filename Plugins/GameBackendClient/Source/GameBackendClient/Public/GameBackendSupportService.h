// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameBackendTypes.h"
#include "GameBackendSupportService.generated.h"

class UGameBackendClientSubsystem;
class FGameBackendHttpClient;

UCLASS(BlueprintType)
class GAMEBACKENDCLIENT_API UGameBackendSupportService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Support")
	void SubmitTicket(const FGameBackendSupportTicketRequest& Request, const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Support")
	void GetMyTickets(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Support")
	void SubmitReport(const FGameBackendReportRequest& Request, const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Support")
	void GetMyReports(const FGameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "GameBackend|Support")
	void SubmitAppeal(const FString& AppealJson, const FGameBackendResponseDelegate& Callback);

private:
	TWeakObjectPtr<UGameBackendClientSubsystem> Subsystem;
	FGameBackendHttpClient* HttpClient = nullptr;
};
