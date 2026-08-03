// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Widgets/Lobby/UDBAQueueStatusWidgetBase.h"

#include "GameDBA/UI/Controllers/DBAGameUIManager.h"

UDBAQueueStatusWidgetBase::UDBAQueueStatusWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ElapsedWaitTime(0.0f)
	, bIsQueuing(false)
{
}

void UDBAQueueStatusWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBAQueueStatusWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBAQueueStatusWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAQueueStatusWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsQueuing)
	{
		ElapsedWaitTime += InDeltaTime;
		UpdateWaitTime(ElapsedWaitTime);
	}
}

void UDBAQueueStatusWidgetBase::StartQueue(const FText& InModeName, const FText& InMapName, const FText& InEstimatedWaitTime)
{
	CachedModeName = InModeName;
	CachedMapName = InMapName;
	CachedEstimatedWaitTime = InEstimatedWaitTime;
	ElapsedWaitTime = 0.0f;
	bIsQueuing = true;

	BP_OnQueueStarted(CachedModeName, CachedMapName, CachedEstimatedWaitTime);
}

void UDBAQueueStatusWidgetBase::CancelQueue()
{
	bIsQueuing = false;
	ElapsedWaitTime = 0.0f;

	BP_OnQueueCancelled();
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->HideQueueStatus();
		}
	}
}

void UDBAQueueStatusWidgetBase::UpdateWaitTime(float InElapsedTime)
{
	int32 Minutes = FMath::FloorToInt(InElapsedTime / 60.0f);
	int32 Seconds = FMath::FloorToInt(InElapsedTime) % 60;

	FText WaitTimeText = FText::FromString(FString::Printf(TEXT("%d:%02d"), Minutes, Seconds));
	BP_OnWaitTimeUpdated(WaitTimeText);
}

