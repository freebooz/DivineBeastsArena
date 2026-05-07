// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/ElementSelect/UDBAFixedSkillGroupPreviewWidgetBase.h"

UDBAFixedSkillGroupPreviewWidgetBase::UDBAFixedSkillGroupPreviewWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentZodiac(EDBAZodiac::None)
	, CurrentElement(EDBAElement::None)
{
}

void UDBAFixedSkillGroupPreviewWidgetBase::SetZodiacAndElement(EDBAZodiac Zodiac, EDBAElement Element)
{
	CurrentZodiac = Zodiac;
	CurrentElement = Element;

	FText PassiveName = FText::FromString(TEXT("榧犵帇涔嬫晱"));
	FText Skill01Name = FText::GetEmpty();
	FText Skill02Name = FText::FromString(TEXT("閲戠浘鎶や綋"));
	FText Skill03Name = FText::GetEmpty();
	FText Skill04Name = FText::GetEmpty();
	FText UltimateName = FText::FromString(TEXT("榧犵帇闄嶄复"));

	int32 ResonanceLevel = 4;
	FText ResonanceDescription = FText::FromString(TEXT("鎺у埗鏃堕棿 +1.0绉掞紝鎶ょ浘鍊?+20%"));

	BP_OnUpdateSkillGroupPreview(
		Zodiac,
		Element,
		PassiveName,
		Skill01Name,
		Skill02Name,
		Skill03Name,
		Skill04Name,
		UltimateName,
		ResonanceLevel,
		ResonanceDescription
	);
}

