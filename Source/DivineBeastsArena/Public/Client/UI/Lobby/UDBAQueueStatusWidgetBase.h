// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBAQueueStatusWidgetBase.generated.h"

/**
 * DBAQueueStatusWidgetBase
 *
 * 匹配状态 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAQueueStatusWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAQueueStatusWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|QueueStatus")
	void StartQueue(const FText& ModeName, const FText& MapName, const FText& EstimatedWaitTime);

	UFUNCTION(BlueprintCallable, Category = "DBA|QueueStatus")
	void CancelQueue();

	UFUNCTION(BlueprintCallable, Category = "DBA|QueueStatus")
	void UpdateWaitTime(float ElapsedTime);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|QueueStatus", meta = (DisplayName = "On Queue Started"))
	void BP_OnQueueStarted(const FText& ModeName, const FText& MapName, const FText& EstimatedWaitTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|QueueStatus", meta = (DisplayName = "On Wait Time Updated"))
	void BP_OnWaitTimeUpdated(const FText& WaitTimeText);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|QueueStatus", meta = (DisplayName = "On Queue Cancelled"))
	void BP_OnQueueCancelled();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueStatus")
	FText CachedModeName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueStatus")
	FText CachedMapName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueStatus")
	FText CachedEstimatedWaitTime;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueStatus")
	float ElapsedWaitTime;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueStatus")
	bool bIsQueuing;
};
