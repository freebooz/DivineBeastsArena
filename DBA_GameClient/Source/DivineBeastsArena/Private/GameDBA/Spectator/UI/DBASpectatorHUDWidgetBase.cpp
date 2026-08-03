// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Spectator/UI/DBASpectatorHUDWidgetBase.h"
#include "GameDBA/Spectator/Components/DBASpectatorComponent.h"
#include "GameDBA/Spectator/DBASpectatorManager.h"
#include "GameDBA/Spectator/UI/DBASpectatorStatusBarWidgetBase.h"
#include "GameDBA/Spectator/UI/DBASpectatorMinimapWidgetBase.h"

UDBASpectatorHUDWidgetBase::UDBASpectatorHUDWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bCachedIsPaused(false)
{
}

void UDBASpectatorHUDWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// 默认暂停提示隐藏
	if (PauseOverlay)
	{
		PauseOverlay->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UDBASpectatorHUDWidgetBase::NativeDestruct()
{
	UnbindSpectatorComponent();
	Super::NativeDestruct();
}

void UDBASpectatorHUDWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!SpectatorComponent.IsValid())
	{
		return;
	}

		FDBAObserverViewTarget NewTarget = SpectatorComponent->GetCurrentViewTarget();
	if (NewTarget.TargetCharacter != CachedViewTarget.TargetCharacter)
	{
		CachedViewTarget = NewTarget;
		OnViewTargetUpdated(NewTarget);

		// 更新当前玩家名称
		if (CurrentPlayerNameText)
		{
			CurrentPlayerNameText->SetText(FText::FromName(NewTarget.PlayerName));
		}
	}

		UDBASpectatorManager* Manager = SpectatorComponent->GetSpectatorManager();
	if (Manager)
	{
		bool bCurrentPauseState = Manager->IsPaused();
		if (bCurrentPauseState != bCachedIsPaused)
		{
			bCachedIsPaused = bCurrentPauseState;
			OnPauseStateChanged(bCachedIsPaused);

			// 更新暂停提示 UI
			if (PauseOverlay)
			{
				PauseOverlay->SetVisibility(bCachedIsPaused ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
			}
			if (PauseIndicatorText)
			{
				PauseIndicatorText->SetText(bCachedIsPaused ? FText::FromString(TEXT("PAUSED")) : FText::GetEmpty());
			}
		}
	}
}

void UDBASpectatorHUDWidgetBase::BindSpectatorComponent(UDBASpectatorComponent* InSpectatorComponent)
{
	UnbindSpectatorComponent();

	SpectatorComponent = InSpectatorComponent;

	if (SpectatorComponent.IsValid())
	{
				CachedViewTarget = SpectatorComponent->GetCurrentViewTarget();
		OnViewTargetUpdated(CachedViewTarget);
	}
}

void UDBASpectatorHUDWidgetBase::UnbindSpectatorComponent()
{
	if (SpectatorComponent.IsValid())
	{
		SpectatorComponent = nullptr;
	}
}

