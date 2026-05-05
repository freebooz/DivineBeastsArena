// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Spectator/UI/DBASpectatorHUDWidgetBase.h"
#include "GameDBA/Spectator/Components/DBASpectatorComponent.h"
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

	// 检查视角目标是否变化
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

	// 检查暂停状态
	UDBASpectatorManager* Manager = SpectatorComponent->GetSpectatorManager();
	if (Manager)
	{
		bool bCurrentPauseState = Manager->IsPaused();
		if (bCurrentPauseState != bCachedIsPaused)
		{
			bCachedIsPaused = bCurrentPauseState;
			OnPauseStateChanged(bCachedIsPaused);

			// 更新暂停提示UI
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
		// 初始化数据
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
