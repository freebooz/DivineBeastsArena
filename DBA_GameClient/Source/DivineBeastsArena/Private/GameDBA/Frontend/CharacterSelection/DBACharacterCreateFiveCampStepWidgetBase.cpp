// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateFiveCampStepWidgetBase.h"

#include "Components/PanelWidget.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateFiveCampCardWidgetBase.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateFiveCampViewModel.h"
#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

void UDBACharacterCreateFiveCampStepWidgetBase::SetWidgetController(UDBACharacterCreateWidgetController* InController)
{
	if (ViewModel)
	{
		ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateFiveCampStepWidgetBase::HandleViewModelChanged);
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
		ViewModel->OnChanged.AddDynamic(this, &UDBACharacterCreateFiveCampStepWidgetBase::HandleViewModelChanged);
		HandleViewModelChanged();
	}
}

bool UDBACharacterCreateFiveCampStepWidgetBase::NextStep()
{
	return Controller && Controller->Next();
}

void UDBACharacterCreateFiveCampStepWidgetBase::BackStep()
{
	if (Controller)
	{
		Controller->Back();
	}
}

void UDBACharacterCreateFiveCampStepWidgetBase::NativeDestruct()
{
	if (ViewModel)
	{
		ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateFiveCampStepWidgetBase::HandleViewModelChanged);
	}
	SpawnedCards.Reset();
	ViewModel = nullptr;
	Controller = nullptr;
	Super::NativeDestruct();
}

void UDBACharacterCreateFiveCampStepWidgetBase::HandleViewModelChanged()
{
	RebuildFiveCampCards();
}

void UDBACharacterCreateFiveCampStepWidgetBase::HandleFiveCampCardClicked(const EDBAFiveCamp FiveCamp)
{
	// 卡片仅提交用户意图；Controller 根据配置表可用性决定是否更新唯一 Draft，并处理异步主题预览。
	if (Controller)
	{
		Controller->SelectFiveCamp(FiveCamp);
	}
}

void UDBACharacterCreateFiveCampStepWidgetBase::RebuildFiveCampCards()
{
	if (!FiveCampCardContainer || !ViewModel || !FiveCampCardClass)
	{
		return;
	}
	FiveCampCardContainer->ClearChildren();
	SpawnedCards.Reset();
	for (const FDBACharacterCreateFiveCampCardModel& Card : ViewModel->GetFiveCampCards())
	{
		UDBACharacterCreateFiveCampCardWidgetBase* CardWidget = CreateWidget<UDBACharacterCreateFiveCampCardWidgetBase>(this, FiveCampCardClass);
		if (!CardWidget)
		{
			continue;
		}
		CardWidget->ApplyCard(Card);
		CardWidget->OnFiveCampClicked.AddDynamic(this, &UDBACharacterCreateFiveCampStepWidgetBase::HandleFiveCampCardClicked);
		FiveCampCardContainer->AddChild(CardWidget);
		SpawnedCards.Add(CardWidget);
	}
}
