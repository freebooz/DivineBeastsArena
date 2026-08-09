// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementCardWidgetBase.h"

#include "Components/Button.h"

void UDBACharacterCreateElementCardWidgetBase::ApplyCard(const FDBACharacterCreateElementCardModel& InCard)
{
	// 卡片不保留元素业务状态，只缓存 ViewModel 投影以响应自身点击。
	Card = InCard;
	if (SelectButton) SelectButton->SetIsEnabled(Card.bIsAvailable);
	BP_OnCardApplied(Card);
}

void UDBACharacterCreateElementCardWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (SelectButton) SelectButton->OnClicked.AddDynamic(this, &UDBACharacterCreateElementCardWidgetBase::HandleClicked);
}

void UDBACharacterCreateElementCardWidgetBase::HandleClicked()
{
	if (Card.bIsAvailable && Card.Element != EDBAElement::None) OnElementClicked.Broadcast(Card.Element);
}
