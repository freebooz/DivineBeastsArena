// Copyright Freebooz Games, Inc. All Rights Reserved.

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

	// 榛樿鏆傚仠鎻愮ず闅愯棌
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

		// 鏇存柊褰撳墠鐜╁鍚嶇О
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

			// 鏇存柊鏆傚仠鎻愮ずUI
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

