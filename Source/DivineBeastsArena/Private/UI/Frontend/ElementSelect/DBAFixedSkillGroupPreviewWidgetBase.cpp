// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Frontend/ElementSelect/DBAFixedSkillGroupPreviewWidgetBase.h"

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

	FText PassiveName = FText::FromString(TEXT("鼠王之敏"));
	FText Skill01Name = FText::FromString(TEXT("金刃斩"));
	FText Skill02Name = FText::FromString(TEXT("金盾护体"));
	FText Skill03Name = FText::FromString(TEXT("金光闪"));
	FText Skill04Name = FText::FromString(TEXT("金锁链"));
	FText UltimateName = FText::FromString(TEXT("鼠王降临"));

	int32 ResonanceLevel = 4;
	FText ResonanceDescription = FText::FromString(TEXT("控制时间 +1.0秒，护盾值 +20%"));

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
