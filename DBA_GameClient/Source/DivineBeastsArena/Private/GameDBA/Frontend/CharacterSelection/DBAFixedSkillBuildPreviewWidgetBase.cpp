// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBAFixedSkillBuildPreviewWidgetBase.h"

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementViewModel.h"
#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

void UDBAFixedSkillBuildPreviewWidgetBase::SetWidgetController(UDBACharacterCreateWidgetController* InController)
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBAFixedSkillBuildPreviewWidgetBase::HandleViewModelChanged);
	Controller = InController;
	if (!Controller) return;
	Controller->BindElementStep();
	ViewModel = Controller->GetElementStepViewModel();
	if (ViewModel)
	{
		ViewModel->OnChanged.AddDynamic(this, &UDBAFixedSkillBuildPreviewWidgetBase::HandleViewModelChanged);
		HandleViewModelChanged();
	}
}

void UDBAFixedSkillBuildPreviewWidgetBase::NativeDestruct()
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBAFixedSkillBuildPreviewWidgetBase::HandleViewModelChanged);
	ViewModel = nullptr;
	Controller = nullptr;
	Super::NativeDestruct();
}

void UDBAFixedSkillBuildPreviewWidgetBase::HandleViewModelChanged()
{
	if (ViewModel) BP_OnFixedSkillBuildChanged(ViewModel->GetFixedSkillBuildPreview());
}
