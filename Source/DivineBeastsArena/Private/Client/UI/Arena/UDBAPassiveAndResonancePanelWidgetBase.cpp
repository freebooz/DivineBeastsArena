// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Arena/DBAPassiveAndResonancePanelWidgetBase.h"

UDBAPassiveAndResonancePanelWidgetBase::UDBAPassiveAndResonancePanelWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAPassiveAndResonancePanelWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBAPassiveAndResonancePanelWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAPassiveAndResonancePanelWidgetBase::UpdatePassiveSkill(int32 SlotIndex, bool bActive)
{
	BP_OnPassiveUpdated(SlotIndex, bActive);
}

void UDBAPassiveAndResonancePanelWidgetBase::UpdateResonanceLevel(int32 Level)
{
	BP_OnResonanceLevelUpdated(Level);
}
