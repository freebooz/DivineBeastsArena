// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Widgets/Lobby/UDBAReadyCheckWidgetBase.h"

UDBAReadyCheckWidgetBase::UDBAReadyCheckWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TotalTimeout(0.0f)
	, CachedRemainingTime(0.0f)
	, bHasAccepted(false)
	, bIsCompleted(false)
{
}

void UDBAReadyCheckWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBAReadyCheckWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBAReadyCheckWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAReadyCheckWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (CachedRemainingTime > 0.0f)
	{
		CachedRemainingTime = FMath::Max(0.0f, CachedRemainingTime - InDeltaTime);
		float Percentage = TotalTimeout > 0.0f ? CachedRemainingTime / TotalTimeout : 0.0f;
		BP_OnTimeUpdated(CachedRemainingTime, Percentage);

		if (CachedRemainingTime <= 0.0f && !bHasAccepted && !bIsCompleted)
		{
			CompleteReadyCheck(false);
		}
	}
}

void UDBAReadyCheckWidgetBase::ShowReadyCheck(const FText& InModeName, const FText& InMapName, float TimeoutSeconds)
{
	CachedModeName = InModeName;
	CachedMapName = InMapName;
	TotalTimeout = TimeoutSeconds;
	CachedRemainingTime = TimeoutSeconds;
	bHasAccepted = false;
	bIsCompleted = false;

	BP_OnReadyCheckShown(CachedModeName, CachedMapName, TimeoutSeconds);
}

void UDBAReadyCheckWidgetBase::AcceptReadyCheck()
{
	CompleteReadyCheck(true);
}

void UDBAReadyCheckWidgetBase::DeclineReadyCheck()
{
	CompleteReadyCheck(false);
}

void UDBAReadyCheckWidgetBase::UpdateRemainingTime(float InRemainingTime)
{
	CachedRemainingTime = InRemainingTime;
}

void UDBAReadyCheckWidgetBase::CompleteReadyCheck(bool bAccepted)
{
	if (bIsCompleted)
	{
		return;
	}

	bIsCompleted = true;
	bHasAccepted = bAccepted;
	CachedRemainingTime = 0.0f;
	BP_OnReadyCheckCompleted(bAccepted);
	OnReadyCheckCompletedEvent.Broadcast(bAccepted);
}
