// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Arena/UDBACCBarWidgetBase.h"

UDBACCBarWidgetBase::UDBACCBarWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACCBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBACCBarWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBACCBarWidgetBase::AddCCEffect(const FString& CCId, float Duration)
{
	BP_OnCCEffectAdded(CCId, Duration);
}

void UDBACCBarWidgetBase::RemoveCCEffect(const FString& CCId)
{
	BP_OnCCEffectRemoved(CCId);
}

void UDBACCBarWidgetBase::ClearAllCCEffects()
{
}
