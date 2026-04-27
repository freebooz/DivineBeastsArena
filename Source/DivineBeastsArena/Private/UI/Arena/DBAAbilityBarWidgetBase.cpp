// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Arena/DBAAbilityBarWidgetBase.h"

UDBAAbilityBarWidgetBase::UDBAAbilityBarWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAAbilityBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBAAbilityBarWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAAbilityBarWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UDBAAbilityBarWidgetBase::UpdateAbility(int32 SlotIndex, float Cooldown, float ManaCost)
{
	bool bOnCooldown = Cooldown > 0.0f;
	BP_OnAbilityUpdated(SlotIndex, Cooldown, ManaCost, bOnCooldown);
}

void UDBAAbilityBarWidgetBase::SetAbilityEnabled(int32 SlotIndex, bool bEnabled)
{
}
