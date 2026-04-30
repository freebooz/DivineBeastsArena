// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Frontend/ElementSelect/DBAElementInfoPanelWidgetBase.h"
#include "Common/Types/DBACommonEnums.h"

UDBAElementInfoPanelWidgetBase::UDBAElementInfoPanelWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentElement(EDBAElement::None)
{
}

void UDBAElementInfoPanelWidgetBase::SetElement(EDBAElement Element)
{
	CurrentElement = Element;

	FText ElementName = FText::FromString(TEXT("金"));
	FText ElementDescription = FText::FromString(TEXT("金属性，锋利坚固，擅长物理攻击和防御"));

	EDBAElement CounterTo = EDBAElement::Wood;
	EDBAElement CounteredBy = EDBAElement::Fire;

	BP_OnUpdateElementInfo(Element, ElementName, ElementDescription, CounterTo, CounteredBy);
}
