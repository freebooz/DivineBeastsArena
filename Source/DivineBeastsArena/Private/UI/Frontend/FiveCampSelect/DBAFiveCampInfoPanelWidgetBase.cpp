// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Frontend/FiveCampSelect/DBAFiveCampInfoPanelWidgetBase.h"
#include "Common/Types/DBACommonEnums.h"

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

	FText FiveCampName = FText::FromString(TEXT("天阵营"));
	FText FiveCampDescription = FText::FromString(TEXT("天界神兽，外观华丽，特效璀璨"));
	FText AppearanceTheme = FText::FromString(TEXT("金色、白色、光明"));
	FText EffectTheme = FText::FromString(TEXT("光明、圣洁"));

	BP_OnUpdateFiveCampInfo(FiveCamp, FiveCampName, FiveCampDescription, AppearanceTheme, EffectTheme);
}
