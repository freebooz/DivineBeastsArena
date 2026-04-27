// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "DBANewbieTaskTrackerWidgetBase.generated.h"

USTRUCT(BlueprintType)
struct FDBANewbieTaskData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|NewbieTask")
	FName TaskId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|NewbieTask")
	FText TaskTitle;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|NewbieTask")
	FText TaskDescription;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|NewbieTask")
	int32 CurrentProgress;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|NewbieTask")
	int32 TargetProgress;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|NewbieTask")
	bool bIsCompleted;

	FDBANewbieTaskData()
		: CurrentProgress(0)
		, TargetProgress(1)
		, bIsCompleted(false)
	{
	}
};

/**
 * DBANewbieTaskTrackerWidgetBase
 *
 * 新手任务追踪器 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBANewbieTaskTrackerWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBANewbieTaskTrackerWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|NewbieTaskTracker")
	void RefreshTaskList();

	UFUNCTION(BlueprintCallable, Category = "DBA|NewbieTaskTracker")
	void UpdateTaskProgress(FName TaskId, int32 CurrentProgress);

	UFUNCTION(BlueprintCallable, Category = "DBA|NewbieTaskTracker")
	void CompleteTask(FName TaskId);

	UFUNCTION(BlueprintCallable, Category = "DBA|NewbieTaskTracker")
	void ToggleCollapse();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|NewbieTaskTracker", meta = (DisplayName = "On Task List Refreshed"))
	void BP_OnTaskListRefreshed(const TArray<FDBANewbieTaskData>& Tasks);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|NewbieTaskTracker", meta = (DisplayName = "On Task Progress Updated"))
	void BP_OnTaskProgressUpdated(FName TaskId, int32 CurrentProgress, int32 TargetProgress);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|NewbieTaskTracker", meta = (DisplayName = "On Task Completed"))
	void BP_OnTaskCompleted(FName TaskId);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|NewbieTaskTracker")
	TArray<FDBANewbieTaskData> TaskList;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|NewbieTaskTracker")
	bool bIsCollapsed;
};
