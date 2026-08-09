// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementInfoPanelWidgetBase.h"

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementViewModel.h"
#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

void UDBACharacterCreateElementInfoPanelWidgetBase::SetWidgetController(UDBACharacterCreateWidgetController* InController)
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateElementInfoPanelWidgetBase::HandleViewModelChanged);
	Controller = InController;
	if (!Controller) return;
	Controller->BindElementStep();
	ViewModel = Controller->GetElementStepViewModel();
	if (ViewModel)
	{
		ViewModel->OnChanged.AddDynamic(this, &UDBACharacterCreateElementInfoPanelWidgetBase::HandleViewModelChanged);
		HandleViewModelChanged();
	}
}

void UDBACharacterCreateElementInfoPanelWidgetBase::NativeDestruct()
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateElementInfoPanelWidgetBase::HandleViewModelChanged);
	ViewModel = nullptr;
	Controller = nullptr;
	Super::NativeDestruct();
}

void UDBACharacterCreateElementInfoPanelWidgetBase::HandleViewModelChanged()
{
	if (!ViewModel) return;
	const EDBAElement Selected = ViewModel->GetSelectedElement();
	for (const FDBACharacterCreateElementCardModel& Card : ViewModel->GetElementCards())
	{
		if (Card.Element == Selected)
		{
			BP_OnElementInfoChanged(Card.Element, Card.DisplayName, Card.Description, Card.bIsAvailable);
			return;
		}
	}
	BP_OnElementInfoChanged(EDBAElement::None, FText::GetEmpty(), FText::GetEmpty(), false);
}
