// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBAZodiacItemWidgetBase.h"

#include "Components/Button.h"

void UDBAZodiacItemWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	// 点击只广播生肖枚举，由父容器再交给 Controller，避免列表项越过 UI 编排层。
	if (SelectButton) SelectButton->OnClicked.AddDynamic(this, &UDBAZodiacItemWidgetBase::HandleClicked);
}

void UDBAZodiacItemWidgetBase::ApplyItem(const FDBAZodiacCreateListItem& InItem)
{
	Item = InItem;
	BP_OnItemApplied(Item);
}

void UDBAZodiacItemWidgetBase::HandleClicked()
{
	if (Item.Zodiac != EDBAZodiac::None) OnZodiacClicked.Broadcast(Item.Zodiac);
}
