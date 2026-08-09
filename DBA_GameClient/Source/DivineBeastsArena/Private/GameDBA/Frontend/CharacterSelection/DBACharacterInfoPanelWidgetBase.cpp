// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterInfoPanelWidgetBase.h"

#include "Components/TextBlock.h"

void UDBACharacterInfoPanelWidgetBase::SetCharacterDetails(const FDBACharacterDetails& InDetails, const bool bPreviewLoading)
{
	CharacterDetails = InDetails;
	bIsPreviewLoading = bPreviewLoading;
	if (NameText) NameText->SetText(FText::FromString(CharacterDetails.Summary.CharacterName));
	if (ZodiacText)
	{
		const UEnum* Enum = StaticEnum<EDBAZodiac>();
		ZodiacText->SetText(Enum ? Enum->GetDisplayNameTextByValue(static_cast<int64>(CharacterDetails.Summary.Zodiac)) : FText::GetEmpty());
	}
	if (LevelText) LevelText->SetText(FText::Format(NSLOCTEXT("DBACharacterInfo", "Level", "等级 {0}"), FText::AsNumber(CharacterDetails.Summary.Level)));
	if (PreviewStatusText) PreviewStatusText->SetText(bIsPreviewLoading ? NSLOCTEXT("DBACharacterInfo", "PreviewLoading", "角色预览加载中") : FText::GetEmpty());
	BP_OnCharacterDetailsChanged(CharacterDetails, bIsPreviewLoading);
}
