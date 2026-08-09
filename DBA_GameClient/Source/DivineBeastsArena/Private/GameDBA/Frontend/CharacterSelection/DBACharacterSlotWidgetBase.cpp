// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterSlotWidgetBase.h"

#include "Components/TextBlock.h"

namespace
{
	FText ZodiacToText(const EDBAZodiac Zodiac)
	{
		if (const UEnum* Enum = StaticEnum<EDBAZodiac>()) return Enum->GetDisplayNameTextByValue(static_cast<int64>(Zodiac));
		return FText::GetEmpty();
	}
}

void UDBACharacterSlotWidgetBase::SetCharacterSummary(const FDBACharacterSummary& InSummary, const bool bInSelected)
{
	CharacterSummary = InSummary;
	bSelected = bInSelected;
	if (NameText) NameText->SetText(FText::FromString(CharacterSummary.CharacterName));
	if (ZodiacText) ZodiacText->SetText(ZodiacToText(CharacterSummary.Zodiac));
	if (LevelText) LevelText->SetText(FText::Format(NSLOCTEXT("DBACharacterSlot", "Level", "等级 {0}"), FText::AsNumber(CharacterSummary.Level)));
	if (LastPlayedText) LastPlayedText->SetText(CharacterSummary.LastUsedTime > 0 ? FText::AsDateTime(FDateTime::FromUnixTimestamp(CharacterSummary.LastUsedTime)) : NSLOCTEXT("DBACharacterSlot", "NoLastPlayed", "尚未进入游戏"));
	if (LocationText) LocationText->SetText(NSLOCTEXT("DBACharacterSlot", "Location", "当前区服"));
	BP_OnSlotChanged(CharacterSummary, bSelected);
}
