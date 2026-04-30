// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Frontend/Loading/DBALoadingScreenWidgetBase.h"
#include "Client/UI/Frontend/Loading/DBALoadingWidgetController.h"

UDBALoadingScreenWidgetBase::UDBALoadingScreenWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentProgress(0.0f)
{
}

void UDBALoadingScreenWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBALoadingScreenWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBALoadingScreenWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UDBALoadingScreenWidgetBase::NativeOnActivated()
{
}

void UDBALoadingScreenWidgetBase::NativeOnDeactivated()
{
}

void UDBALoadingScreenWidgetBase::SetWidgetController(UDBALoadingWidgetController* InController)
{
	WidgetController = InController;
}

void UDBALoadingScreenWidgetBase::UpdateLoadingProgress(float Progress)
{
	CurrentProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
	BP_OnLoadingProgressUpdated(CurrentProgress);
}

void UDBALoadingScreenWidgetBase::ShowTips(const FText& TipsText)
{
	CurrentTips = TipsText;
	BP_OnTipsUpdated(TipsText);
}
