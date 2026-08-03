// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Widgets/Lobby/UDBANewbieTaskTrackerWidgetBase.h"

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
	Task1.TaskTitle = FText::FromString(TEXT("鐔熸倝鎿嶄綔"));
	Task1.TaskDescription = FText::FromString(TEXT("浣跨敤 WASD 绉诲姩瑙掕壊"));
	Task1.CurrentProgress = 1;
	Task1.TargetProgress = 1;
	Task1.bIsCompleted = true;
	TaskList.Add(Task1);

	FDBANewbieTaskData Task2;
	Task2.TaskId = FName(TEXT("NewbieTask_002"));
	Task2.TaskTitle = FText::GetEmpty();
	Task2.TaskDescription = FText::GetEmpty();
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

