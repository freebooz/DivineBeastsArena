// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/FiveCampSelect/DBAFiveCampSelectWidgetBase.h"
#include "Client/UI/Lobby/FiveCampSelect/DBAFiveCampInfoPanelWidgetBase.h"
#include "Client/UI/Lobby/FiveCampSelect/DBAFiveCampSelectWidgetController.h"

UDBAFiveCampSelectWidgetBase::UDBAFiveCampSelectWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SelectedZodiac(EDBAZodiac::None)
	, SelectedElement(EDBAElement::None)
	, CurrentSelectedFiveCamp(EDBAFiveCamp::None)
	, bHasSelectedFiveCamp(false)
{
}

void UDBAFiveCampSelectWidgetBase::NativeConstruct()
{
	// UDBAWidgetBase does not have NativeConstruct/NativeDestruct/NativeOnActivated/NativeOnDeactivated
	// The base class is UObject, not UUserWidget
}

void UDBAFiveCampSelectWidgetBase::NativeDestruct()
{
}

void UDBAFiveCampSelectWidgetBase::NativeOnActivated()
{
	RefreshFiveCampList();
}

void UDBAFiveCampSelectWidgetBase::NativeOnDeactivated()
{
	CurrentSelectedFiveCamp = EDBAFiveCamp::None;
	bHasSelectedFiveCamp = false;
}

void UDBAFiveCampSelectWidgetBase::SetWidgetController(UDBAFiveCampSelectWidgetController* InController)
{
	WidgetController = InController;
}

void UDBAFiveCampSelectWidgetBase::SetSelectedZodiacAndElement(EDBAZodiac Zodiac, EDBAElement Element)
{
	SelectedZodiac = Zodiac;
	SelectedElement = Element;
}

void UDBAFiveCampSelectWidgetBase::RefreshFiveCampList()
{
	TArray<EDBAFiveCamp> AvailableFiveCamps;
	AvailableFiveCamps.Add(EDBAFiveCamp::Center);
	AvailableFiveCamps.Add(EDBAFiveCamp::East);
	AvailableFiveCamps.Add(EDBAFiveCamp::West);
	AvailableFiveCamps.Add(EDBAFiveCamp::South);
	AvailableFiveCamps.Add(EDBAFiveCamp::North);

	BP_OnRefreshFiveCampList(AvailableFiveCamps);
}

void UDBAFiveCampSelectWidgetBase::SelectFiveCamp(EDBAFiveCamp FiveCamp)
{
	if (FiveCamp == EDBAFiveCamp::None)
	{
		return;
	}

	CurrentSelectedFiveCamp = FiveCamp;
	bHasSelectedFiveCamp = true;

	BP_OnFiveCampSelected(FiveCamp);
	BP_OnConfirmButtonStateChanged(true);

	if (FiveCampInfoPanel)
	{
		FiveCampInfoPanel->SetFiveCamp(FiveCamp, SelectedZodiac, SelectedElement);
	}
}

void UDBAFiveCampSelectWidgetBase::ConfirmFiveCampSelection()
{
	if (!bHasSelectedFiveCamp || !WidgetController)
	{
		return;
	}

	WidgetController->ConfirmFiveCampSelection(CurrentSelectedFiveCamp);
}

void UDBAFiveCampSelectWidgetBase::OnBackButtonClicked()
{
	if (WidgetController)
	{
		WidgetController->RequestBack();
	}
}
