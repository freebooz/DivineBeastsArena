// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/FiveCampSelect/UDBAFiveCampInfoPanelWidgetBase.h"
#include "GameCore/Types/DBACommonEnums.h"

UDBAFiveCampInfoPanelWidgetBase::UDBAFiveCampInfoPanelWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentFiveCamp(EDBAFiveCamp::None)
	, CurrentZodiac(EDBAZodiac::None)
	, CurrentElement(EDBAElement::None)
{
}

void UDBAFiveCampInfoPanelWidgetBase::SetFiveCamp(EDBAFiveCamp FiveCamp, EDBAZodiac Zodiac, EDBAElement Element)
{
	CurrentFiveCamp = FiveCamp;
	CurrentZodiac = Zodiac;
	CurrentElement = Element;

	FText FiveCampName = FText::GetEmpty();
	FText FiveCampDescription = FText::GetEmpty();
	FText AppearanceTheme = FText::GetEmpty();
	FText EffectTheme = FText::GetEmpty();

	BP_OnUpdateFiveCampInfo(FiveCamp, FiveCampName, FiveCampDescription, AppearanceTheme, EffectTheme);
}

