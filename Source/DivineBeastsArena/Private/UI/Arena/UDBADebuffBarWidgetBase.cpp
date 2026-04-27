// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Arena/UDBADebuffBarWidgetBase.h"

UDBADebuffBarWidgetBase::UDBADebuffBarWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBADebuffBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBADebuffBarWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBADebuffBarWidgetBase::AddDebuff(const FString& DebuffId, float Duration)
{
	BP_OnDebuffAdded(DebuffId, Duration);
}

void UDBADebuffBarWidgetBase::RemoveDebuff(const FString& DebuffId)
{
	BP_OnDebuffRemoved(DebuffId);
}

void UDBADebuffBarWidgetBase::ClearAllDebuffs()
{
}
