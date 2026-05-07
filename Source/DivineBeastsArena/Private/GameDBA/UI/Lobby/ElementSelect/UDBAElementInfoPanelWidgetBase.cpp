// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/ElementSelect/UDBAElementInfoPanelWidgetBase.h"
#include "GameCore/Types/DBACommonEnums.h"

UDBAElementInfoPanelWidgetBase::UDBAElementInfoPanelWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentElement(EDBAElement::None)
{
}

void UDBAElementInfoPanelWidgetBase::SetElement(EDBAElement Element)
{
	CurrentElement = Element;

	FText ElementName = FText::GetEmpty();
	FText ElementDescription = FText::FromString(TEXT("閲戝睘鎬э紝閿嬪埄鍧氬浐锛屾搮闀跨墿鐞嗘敾鍑诲拰闃插尽"));

	EDBAElement CounterTo = EDBAElement::Wood;
	EDBAElement CounteredBy = EDBAElement::Fire;

	BP_OnUpdateElementInfo(Element, ElementName, ElementDescription, CounterTo, CounteredBy);
}

