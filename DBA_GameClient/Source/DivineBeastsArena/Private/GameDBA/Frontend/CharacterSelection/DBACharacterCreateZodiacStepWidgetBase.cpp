// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateZodiacStepWidgetBase.h"

#include "Components/PanelWidget.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateZodiacViewModel.h"
#include "GameDBA/Frontend/CharacterSelection/DBAZodiacItemWidgetBase.h"
#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateWidgetController.h"

void UDBACharacterCreateZodiacStepWidgetBase::SetWidgetController(UDBACharacterCreateWidgetController* InController)
{
	// 先解绑旧 ViewModel，避免 Widget 重用时重复订阅并生成两套生肖列表。
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateZodiacStepWidgetBase::HandleViewModelChanged);
	Controller = InController;
	if (!Controller) return;
	Controller->BindZodiacStep();
	ViewModel = Controller->GetZodiacStepViewModel();
	if (ViewModel)
	{
		ViewModel->OnChanged.AddDynamic(this, &UDBACharacterCreateZodiacStepWidgetBase::HandleViewModelChanged);
		RebuildZodiacItems();
	}
}

void UDBACharacterCreateZodiacStepWidgetBase::NextStep()
{
	// Controller/Flow 负责状态转移；Widget 仅请求继续，不直接替换 Screen。
	if (Controller) Controller->Next();
}

void UDBACharacterCreateZodiacStepWidgetBase::NativeDestruct()
{
	if (ViewModel) ViewModel->OnChanged.RemoveDynamic(this, &UDBACharacterCreateZodiacStepWidgetBase::HandleViewModelChanged);
	ViewModel = nullptr;
	Controller = nullptr;
	Super::NativeDestruct();
}

void UDBACharacterCreateZodiacStepWidgetBase::HandleViewModelChanged()
{
	RebuildZodiacItems();
}

void UDBACharacterCreateZodiacStepWidgetBase::HandleZodiacClicked(const EDBAZodiac Zodiac)
{
	if (Controller) Controller->SelectZodiac(Zodiac);
}

void UDBACharacterCreateZodiacStepWidgetBase::RebuildZodiacItems()
{
	if (!ZodiacListContainer || !ZodiacItemWidgetClass || !ViewModel) return;
	ZodiacListContainer->ClearChildren();
	for (const FDBAZodiacCreateListItem& Item : ViewModel->GetZodiacItems())
	{
		UDBAZodiacItemWidgetBase* Widget = CreateWidget<UDBAZodiacItemWidgetBase>(GetOwningPlayer(), ZodiacItemWidgetClass);
		if (!Widget) continue;
		Widget->ApplyItem(Item);
		Widget->OnZodiacClicked.AddDynamic(this, &UDBACharacterCreateZodiacStepWidgetBase::HandleZodiacClicked);
		ZodiacListContainer->AddChild(Widget);
	}
}
