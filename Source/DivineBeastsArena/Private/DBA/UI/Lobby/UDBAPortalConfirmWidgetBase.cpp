// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/UDBAPortalConfirmWidgetBase.h"

UDBAPortalConfirmWidgetBase::UDBAPortalConfirmWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CachedCanTeleport(false)
{
}

void UDBAPortalConfirmWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBAPortalConfirmWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBAPortalConfirmWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAPortalConfirmWidgetBase::ShowConfirm(FName InDestinationId, const FText& InDestinationName, const FText& InDestinationDescription, bool bInCanTeleport, const FText& InConditionText)
{
	DestinationId = InDestinationId;
	CachedDestinationName = InDestinationName;
	CachedDestinationDescription = InDestinationDescription;
	CachedCanTeleport = bInCanTeleport;
	CachedConditionText = InConditionText;

	BP_OnShowConfirm(InDestinationName, InDestinationDescription, bInCanTeleport, InConditionText);
}

void UDBAPortalConfirmWidgetBase::ConfirmTeleport()
{
	if (!CachedCanTeleport)
	{
		return;
	}

	OnPortalConfirmedEvent.Broadcast(DestinationId);
	RemoveFromParent();
}

void UDBAPortalConfirmWidgetBase::CancelTeleport()
{
	OnPortalCancelledEvent.Broadcast();
	RemoveFromParent();
}
