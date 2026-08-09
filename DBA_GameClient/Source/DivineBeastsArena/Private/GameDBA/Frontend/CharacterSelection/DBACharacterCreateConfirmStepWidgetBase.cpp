// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateConfirmStepWidgetBase.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateConfirmViewModel.h"
#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

void UDBACharacterCreateConfirmStepWidgetBase::SetWidgetController(UDBACharacterCreateWidgetController* InController)
{
	if (ViewModel)
	{
		ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateConfirmStepWidgetBase::HandleViewModelChanged);
	}
	Controller = InController;
	if (!Controller)
	{
		return;
	}
	Controller->BindConfirmStep();
	ViewModel = Controller->GetConfirmStepViewModel();
	if (ViewModel)
	{
		ViewModel->OnChanged.AddDynamic(this, &UDBACharacterCreateConfirmStepWidgetBase::HandleViewModelChanged);
		if (CharacterNameInput)
		{
			CharacterNameInput->SetText(FText::FromString(ViewModel->GetCharacterName()));
		}
		HandleViewModelChanged();
	}
}

void UDBACharacterCreateConfirmStepWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (CharacterNameInput) CharacterNameInput->OnTextChanged.AddDynamic(this, &UDBACharacterCreateConfirmStepWidgetBase::HandleNameChanged);
	if (CreateButton) CreateButton->OnClicked.AddDynamic(this, &UDBACharacterCreateConfirmStepWidgetBase::HandleCreateClicked);
	if (BackButton) BackButton->OnClicked.AddDynamic(this, &UDBACharacterCreateConfirmStepWidgetBase::HandleBackClicked);
	if (CancelButton) CancelButton->OnClicked.AddDynamic(this, &UDBACharacterCreateConfirmStepWidgetBase::HandleCancelClicked);
}

void UDBACharacterCreateConfirmStepWidgetBase::NativeDestruct()
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateConfirmStepWidgetBase::HandleViewModelChanged);
	if (CharacterNameInput) CharacterNameInput->OnTextChanged.RemoveDynamic(this, &UDBACharacterCreateConfirmStepWidgetBase::HandleNameChanged);
	if (CreateButton) CreateButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateConfirmStepWidgetBase::HandleCreateClicked);
	if (BackButton) BackButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateConfirmStepWidgetBase::HandleBackClicked);
	if (CancelButton) CancelButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateConfirmStepWidgetBase::HandleCancelClicked);
	ViewModel = nullptr;
	Controller = nullptr;
	Super::NativeDestruct();
}

void UDBACharacterCreateConfirmStepWidgetBase::SubmitCreate()
{
	if (Controller)
	{
		Controller->SubmitConfirmedCharacterCreation();
	}
}

void UDBACharacterCreateConfirmStepWidgetBase::BackStep()
{
	if (Controller)
	{
		Controller->Back();
	}
}

void UDBACharacterCreateConfirmStepWidgetBase::CancelSubmission()
{
	if (Controller)
	{
		Controller->CancelConfirmedCharacterCreation();
	}
}

void UDBACharacterCreateConfirmStepWidgetBase::HandleNameChanged(const FText& NewName)
{
	// 输入只表达用户意图；Draft 会广播变化并驱动全部摘要重新投影，Widget 不保留第二份角色名状态。
	if (Controller)
	{
		Controller->SetCharacterName(NewName.ToString());
	}
}

void UDBACharacterCreateConfirmStepWidgetBase::HandleCreateClicked()
{
	SubmitCreate();
}

void UDBACharacterCreateConfirmStepWidgetBase::HandleBackClicked()
{
	BackStep();
}

void UDBACharacterCreateConfirmStepWidgetBase::HandleCancelClicked()
{
	CancelSubmission();
}

void UDBACharacterCreateConfirmStepWidgetBase::HandleViewModelChanged()
{
	if (!ViewModel)
	{
		return;
	}
	if (CreateButton)
	{
		CreateButton->SetIsEnabled(!ViewModel->IsSubmitting());
	}
	BP_OnConfirmViewModelChanged(ViewModel);
}
