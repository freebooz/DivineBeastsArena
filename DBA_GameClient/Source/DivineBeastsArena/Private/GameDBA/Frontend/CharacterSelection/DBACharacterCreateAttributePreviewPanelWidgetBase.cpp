// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateAttributePreviewPanelWidgetBase.h"

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementViewModel.h"
#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

void UDBACharacterCreateAttributePreviewPanelWidgetBase::SetWidgetController(UDBACharacterCreateWidgetController* InController)
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateAttributePreviewPanelWidgetBase::HandleViewModelChanged);
	Controller = InController;
	if (!Controller) return;
	Controller->BindElementStep();
	ViewModel = Controller->GetElementStepViewModel();
	if (ViewModel)
	{
		ViewModel->OnChanged.AddDynamic(this, &UDBACharacterCreateAttributePreviewPanelWidgetBase::HandleViewModelChanged);
		HandleViewModelChanged();
	}
}

void UDBACharacterCreateAttributePreviewPanelWidgetBase::NativeDestruct()
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateAttributePreviewPanelWidgetBase::HandleViewModelChanged);
	ViewModel = nullptr;
	Controller = nullptr;
	Super::NativeDestruct();
}

void UDBACharacterCreateAttributePreviewPanelWidgetBase::HandleViewModelChanged()
{
	if (ViewModel) BP_OnAttributePreviewChanged(ViewModel->GetAttributePreview());
}
