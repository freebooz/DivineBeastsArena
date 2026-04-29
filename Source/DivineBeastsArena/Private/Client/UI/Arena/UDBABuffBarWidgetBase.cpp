// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Arena/UDBABuffBarWidgetBase.h"

UDBABuffBarWidgetBase::UDBABuffBarWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBABuffBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBABuffBarWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBABuffBarWidgetBase::AddBuff(const FString& BuffId, float Duration)
{
	BP_OnBuffAdded(BuffId, Duration);
}

void UDBABuffBarWidgetBase::RemoveBuff(const FString& BuffId)
{
	BP_OnBuffRemoved(BuffId);
}

void UDBABuffBarWidgetBase::ClearAllBuffs()
{
}
