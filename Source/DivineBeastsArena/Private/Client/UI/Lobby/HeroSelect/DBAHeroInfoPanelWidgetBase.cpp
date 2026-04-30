// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/HeroSelect/DBAHeroInfoPanelWidgetBase.h"

UDBAHeroInfoPanelWidgetBase::UDBAHeroInfoPanelWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentZodiac(EDBAZodiac::None)
{
}

void UDBAHeroInfoPanelWidgetBase::SetZodiac(EDBAZodiac Zodiac)
{
	CurrentZodiac = Zodiac;

	FText ZodiacName = FText::FromString(TEXT("子鼠"));
	FText ZodiacDescription = FText::FromString(TEXT("灵活敏捷的生肖，擅长快速移动和连续攻击"));
	FText UltimateDescription = FText::FromString(TEXT("鼠王降临：召唤鼠群对范围内敌人造成持续伤害"));

	BP_OnUpdateZodiacInfo(Zodiac, ZodiacName, ZodiacDescription, UltimateDescription);
}
