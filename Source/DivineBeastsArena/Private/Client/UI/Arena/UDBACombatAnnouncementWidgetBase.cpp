// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Arena/UDBACombatAnnouncementWidgetBase.h"

UDBACombatAnnouncementWidgetBase::UDBACombatAnnouncementWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACombatAnnouncementWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBACombatAnnouncementWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBACombatAnnouncementWidgetBase::ShowAnnouncement(const FText& Text, float Duration)
{
}

void UDBACombatAnnouncementWidgetBase::ClearAnnouncement()
{
}
