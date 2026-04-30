// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/UDBANewbieTaskTrackerWidgetBase.h"

UDBANewbieTaskTrackerWidgetBase::UDBANewbieTaskTrackerWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsCollapsed(false)
{
}

void UDBANewbieTaskTrackerWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBANewbieTaskTrackerWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshTaskList();
}

void UDBANewbieTaskTrackerWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBANewbieTaskTrackerWidgetBase::RefreshTaskList()
{
	TaskList.Empty();

	FDBANewbieTaskData Task1;
	Task1.TaskId = FName(TEXT("NewbieTask_001"));
	Task1.TaskTitle = FText::FromString(TEXT("熟悉操作"));
	Task1.TaskDescription = FText::FromString(TEXT("使用 WASD 移动角色"));
	Task1.CurrentProgress = 1;
	Task1.TargetProgress = 1;
	Task1.bIsCompleted = true;
	TaskList.Add(Task1);

	FDBANewbieTaskData Task2;
	Task2.TaskId = FName(TEXT("NewbieTask_002"));
	Task2.TaskTitle = FText::FromString(TEXT("释放技能"));
	Task2.TaskDescription = FText::FromString(TEXT("使用技能攻击训练假人"));
	Task2.CurrentProgress = 3;
	Task2.TargetProgress = 5;
	Task2.bIsCompleted = false;
	TaskList.Add(Task2);

	BP_OnTaskListRefreshed(TaskList);
}

void UDBANewbieTaskTrackerWidgetBase::UpdateTaskProgress(FName TaskId, int32 CurrentProgress)
{
	FDBANewbieTaskData* FoundTask = TaskList.FindByPredicate([&TaskId](const FDBANewbieTaskData& Task)
	{
		return Task.TaskId == TaskId;
	});

	if (FoundTask)
	{
		FoundTask->CurrentProgress = CurrentProgress;
		BP_OnTaskProgressUpdated(TaskId, CurrentProgress, FoundTask->TargetProgress);

		if (CurrentProgress >= FoundTask->TargetProgress)
		{
			CompleteTask(TaskId);
		}
	}
}

void UDBANewbieTaskTrackerWidgetBase::CompleteTask(FName TaskId)
{
	FDBANewbieTaskData* FoundTask = TaskList.FindByPredicate([&TaskId](const FDBANewbieTaskData& Task)
	{
		return Task.TaskId == TaskId;
	});

	if (FoundTask)
	{
		FoundTask->bIsCompleted = true;
		BP_OnTaskCompleted(TaskId);
	}
}

void UDBANewbieTaskTrackerWidgetBase::ToggleCollapse()
{
	bIsCollapsed = !bIsCollapsed;
}
