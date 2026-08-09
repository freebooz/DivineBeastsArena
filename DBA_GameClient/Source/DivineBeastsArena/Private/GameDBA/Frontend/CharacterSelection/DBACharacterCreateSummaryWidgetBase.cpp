// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateSummaryWidgetBase.h"

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateConfirmViewModel.h"
#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

void UDBACharacterCreateSummaryWidgetBase::SetWidgetController(UDBACharacterCreateWidgetController* InController)
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateSummaryWidgetBase::HandleViewModelChanged);
	Controller = InController;
	if (!Controller) return;
	Controller->BindConfirmStep();
	ViewModel = Controller->GetConfirmStepViewModel();
	if (ViewModel)
	{
		ViewModel->OnChanged.AddDynamic(this, &UDBACharacterCreateSummaryWidgetBase::HandleViewModelChanged);
		HandleViewModelChanged();
	}
}

void UDBACharacterCreateSummaryWidgetBase::NativeDestruct()
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateSummaryWidgetBase::HandleViewModelChanged);
	ViewModel = nullptr;
	Controller = nullptr;
	Super::NativeDestruct();
}

void UDBACharacterCreateSummaryWidgetBase::HandleViewModelChanged()
{
	if (ViewModel) BP_OnSummaryChanged(ViewModel);
}
