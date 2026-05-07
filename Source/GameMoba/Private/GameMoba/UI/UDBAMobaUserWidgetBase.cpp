// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"

#include "GameFramework/PlayerController.h"

UDBAMobaUserWidgetBase::UDBAMobaUserWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAMobaUserWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	OwnerPlayerController = GetOwningPlayer();
}

void UDBAMobaUserWidgetBase::NativeDestruct()
{
	OwnerPlayerController.Reset();
	Super::NativeDestruct();
}

void UDBAMobaUserWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}
