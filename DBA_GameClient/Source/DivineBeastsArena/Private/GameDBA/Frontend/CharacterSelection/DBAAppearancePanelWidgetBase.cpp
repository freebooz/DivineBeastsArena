// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBAAppearancePanelWidgetBase.h"

#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

void UDBAAppearancePanelWidgetBase::SetWidgetController(UDBACharacterCreateWidgetController* InController)
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBAAppearancePanelWidgetBase::HandleViewModelChanged);
	Controller = InController;
	if (!Controller) return;
	Controller->BindZodiacStep();
	ViewModel = Controller->GetZodiacStepViewModel();
	if (ViewModel)
	{
		ViewModel->OnChanged.AddDynamic(this, &UDBAAppearancePanelWidgetBase::HandleViewModelChanged);
		HandleViewModelChanged();
	}
}

bool UDBAAppearancePanelWidgetBase::SelectOption(const EDBAAppearanceSlot AppearanceSlot, const FName OptionId)
{
	// 所有合法性由 Draft 在写入前验证，Panel 不维护自己的允许列表或外观路径。
	return Controller && Controller->SelectAppearanceOption(AppearanceSlot, OptionId);
}

bool UDBAAppearancePanelWidgetBase::Randomize()
{
	return Controller && Controller->RandomizeAppearance();
}

bool UDBAAppearancePanelWidgetBase::ResetToDefault()
{
	return Controller && Controller->ResetAppearance();
}

void UDBAAppearancePanelWidgetBase::NativeDestruct()
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBAAppearancePanelWidgetBase::HandleViewModelChanged);
	ViewModel = nullptr;
	Controller = nullptr;
	Super::NativeDestruct();
}

void UDBAAppearancePanelWidgetBase::HandleViewModelChanged()
{
	if (ViewModel) BP_OnAppearanceGroupsChanged(ViewModel->GetAppearanceGroups());
}
