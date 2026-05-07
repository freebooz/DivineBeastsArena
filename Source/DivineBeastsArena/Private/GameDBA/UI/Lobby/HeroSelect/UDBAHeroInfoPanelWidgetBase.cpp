// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/HeroSelect/UDBAHeroInfoPanelWidgetBase.h"

UDBAHeroInfoPanelWidgetBase::UDBAHeroInfoPanelWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentZodiac(EDBAZodiac::None)
{
}

void UDBAHeroInfoPanelWidgetBase::SetZodiac(EDBAZodiac Zodiac)
{
	CurrentZodiac = Zodiac;

	FText ZodiacName = FText::FromString(TEXT("瀛愰紶"));
	FText ZodiacDescription = FText::FromString(TEXT("鐏垫椿鏁忔嵎鐨勭敓鑲栵紝鎿呴暱蹇€熺Щ鍔ㄥ拰杩炵画鏀诲嚮"));
	FText UltimateDescription = FText::FromString(TEXT("榧犵帇闄嶄复锛氬彫鍞ら紶缇ゅ鑼冨洿鍐呮晫浜洪€犳垚鎸佺画浼ゅ"));

	BP_OnUpdateZodiacInfo(Zodiac, ZodiacName, ZodiacDescription, UltimateDescription);
}

