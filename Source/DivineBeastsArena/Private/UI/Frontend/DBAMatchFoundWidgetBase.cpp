// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Frontend/DBAMatchFoundWidgetBase.h"

UDBAMatchFoundWidgetBase::UDBAMatchFoundWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAMatchFoundWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBAMatchFoundWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBAMatchFoundWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();

	if (AutoNavigateTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(AutoNavigateTimerHandle);
		}
	}
}

void UDBAMatchFoundWidgetBase::ShowMatchFound(const FText& InModeName, const FText& InMapName)
{
	CachedModeName = InModeName;
	CachedMapName = InMapName;

	BP_OnMatchFoundShown(CachedModeName, CachedMapName);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AutoNavigateTimerHandle, this, &UDBAMatchFoundWidgetBase::AutoNavigateToReadyCheck, 2.0f, false);
	}
}

void UDBAMatchFoundWidgetBase::AutoNavigateToReadyCheck()
{
}
