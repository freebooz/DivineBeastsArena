// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetController.h"
#include "Frontend/Queue/DBAQueueSubsystem.h"
#include "DBAQueueWidgetController.generated.h"

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAQueueWidgetController : public UDBAWidgetController
{
	GENERATED_BODY()

public:
	UDBAQueueWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestJoinQueue(EDBAQueueSubsystemMode Mode);

	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestLeaveQueue();

	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestAcceptMatch();

	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestDeclineMatch();

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueueStateChanged, EDBAQueueSubsystemState, NewState);
	UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
	FOnQueueStateChanged OnQueueStateChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchFound, const FDBAQueueSubsystemMatchInfo&, MatchInfo);
	UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
	FOnMatchFound OnMatchFound;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueueCancelled, const FString&, Reason);
	UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
	FOnQueueCancelled OnQueueCancelled;
};
