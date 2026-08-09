// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateFiveCampInfoPanelWidgetBase.h"

#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

void UDBACharacterCreateFiveCampInfoPanelWidgetBase::SetWidgetController(UDBACharacterCreateWidgetController* InController)
{
	if (ViewModel)
	{
		ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateFiveCampInfoPanelWidgetBase::HandleViewModelChanged);
	}
	Controller = InController;
	if (!Controller)
	{
		return;
	}
	Controller->BindFiveCampStep();
	ViewModel = Controller->GetFiveCampStepViewModel();
	if (ViewModel)
	{
		ViewModel->OnChanged.AddDynamic(this, &UDBACharacterCreateFiveCampInfoPanelWidgetBase::HandleViewModelChanged);
		HandleViewModelChanged();
	}
}

void UDBACharacterCreateFiveCampInfoPanelWidgetBase::NativeDestruct()
{
	if (ViewModel)
	{
		ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateFiveCampInfoPanelWidgetBase::HandleViewModelChanged);
	}
	ViewModel = nullptr;
	Controller = nullptr;
	Super::NativeDestruct();
}

void UDBACharacterCreateFiveCampInfoPanelWidgetBase::HandleViewModelChanged()
{
	if (ViewModel)
	{
		BP_OnFiveCampInfoChanged(ViewModel->GetSelectedCard(), ViewModel->GetValidationMessage());
	}
}
