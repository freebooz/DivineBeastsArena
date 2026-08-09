// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateFiveCampCardWidgetBase.h"

#include "Components/Button.h"

void UDBACharacterCreateFiveCampCardWidgetBase::ApplyCard(const FDBACharacterCreateFiveCampCardModel& InCard)
{
	// 可用性完全来自数据表和 Controller 的结构化校验，Widget 不推导任何业务规则。
	Card = InCard;
	if (SelectButton)
	{
		SelectButton->SetIsEnabled(Card.bIsAvailable);
	}
	BP_OnCardApplied(Card);
}

void UDBACharacterCreateFiveCampCardWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &UDBACharacterCreateFiveCampCardWidgetBase::HandleClicked);
	}
}

void UDBACharacterCreateFiveCampCardWidgetBase::HandleClicked()
{
	if (Card.bIsAvailable && Card.FiveCamp != EDBAFiveCamp::None)
	{
		OnFiveCampClicked.Broadcast(Card.FiveCamp);
	}
}
