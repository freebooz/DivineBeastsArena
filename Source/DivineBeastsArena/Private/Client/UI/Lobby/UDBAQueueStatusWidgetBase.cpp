// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/DBAQueueStatusWidgetBase.h"

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
}

void UDBAQueueStatusWidgetBase::UpdateWaitTime(float InElapsedTime)
{
	int32 Minutes = FMath::FloorToInt(InElapsedTime / 60.0f);
	int32 Seconds = FMath::FloorToInt(InElapsedTime) % 60;

	FText WaitTimeText = FText::FromString(FString::Printf(TEXT("%d 分 %02d 秒"), Minutes, Seconds));
	BP_OnWaitTimeUpdated(WaitTimeText);
}
