// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementStepWidgetBase.h"

#include "Components/PanelWidget.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementCardWidgetBase.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementViewModel.h"
#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

void UDBACharacterCreateElementStepWidgetBase::SetWidgetController(UDBACharacterCreateWidgetController* InController)
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateElementStepWidgetBase::HandleViewModelChanged);
	Controller = InController;
	if (!Controller) return;
	Controller->BindElementStep();
	ViewModel = Controller->GetElementStepViewModel();
	if (ViewModel)
	{
		ViewModel->OnChanged.AddDynamic(this, &UDBACharacterCreateElementStepWidgetBase::HandleViewModelChanged);
		HandleViewModelChanged();
	}
}

bool UDBACharacterCreateElementStepWidgetBase::NextStep()
{
	return Controller && Controller->Next();
}

void UDBACharacterCreateElementStepWidgetBase::BackStep()
{
	if (Controller) Controller->Back();
}

void UDBACharacterCreateElementStepWidgetBase::NativeDestruct()
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateElementStepWidgetBase::HandleViewModelChanged);
	SpawnedCards.Reset();
	ViewModel = nullptr;
	Controller = nullptr;
	Super::NativeDestruct();
}

void UDBACharacterCreateElementStepWidgetBase::HandleViewModelChanged()
{
	RebuildElementCards();
}

void UDBACharacterCreateElementStepWidgetBase::HandleElementCardClicked(const EDBAElement Element)
{
	// 卡片只提交玩家意图；Controller 根据固定技能组可用性决定是否写入 Draft。
	if (Controller) Controller->SelectElement(Element);
}

void UDBACharacterCreateElementStepWidgetBase::RebuildElementCards()
{
	if (!ElementCardContainer || !ViewModel || !ElementCardClass) return;
	ElementCardContainer->ClearChildren();
	SpawnedCards.Reset();
	for (const FDBACharacterCreateElementCardModel& Card : ViewModel->GetElementCards())
	{
		UDBACharacterCreateElementCardWidgetBase* CardWidget = CreateWidget<UDBACharacterCreateElementCardWidgetBase>(this, ElementCardClass);
		if (!CardWidget) continue;
		CardWidget->ApplyCard(Card);
		CardWidget->OnElementClicked.AddDynamic(this, &UDBACharacterCreateElementStepWidgetBase::HandleElementCardClicked);
		ElementCardContainer->AddChild(CardWidget);
		SpawnedCards.Add(CardWidget);
	}
}
