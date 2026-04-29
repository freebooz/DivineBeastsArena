// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Frontend/DBAReadyCheckWidgetBase.h"

UDBAReadyCheckWidgetBase::UDBAReadyCheckWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TotalTimeout(0.0f)
	, CachedRemainingTime(0.0f)
	, bHasAccepted(false)
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

		if (CachedRemainingTime <= 0.0f && !bHasAccepted)
		{
			BP_OnReadyCheckCompleted(false);
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

	BP_OnReadyCheckShown(CachedModeName, CachedMapName, TimeoutSeconds);
}

void UDBAReadyCheckWidgetBase::AcceptReadyCheck()
{
	bHasAccepted = true;
	BP_OnReadyCheckCompleted(true);
}

void UDBAReadyCheckWidgetBase::DeclineReadyCheck()
{
	bHasAccepted = false;
	BP_OnReadyCheckCompleted(false);
}

void UDBAReadyCheckWidgetBase::UpdateRemainingTime(float InRemainingTime)
{
	CachedRemainingTime = InRemainingTime;
}
