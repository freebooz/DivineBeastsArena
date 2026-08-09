// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateZodiacViewModel.h"

#include "GameDBA/Data/Assets/DBAZodiacHeroDataAsset.h"
#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"

void UDBACharacterCreateZodiacViewModel::SetAvailableZodiacs(const TArray<EDBAZodiac>& Zodiacs)
{
	// 仅由 Registry 的索引结果创建列表；禁止在 UI 中写死十二个生肖按钮或资产路径。
	ZodiacItems.Reset();
	for (const EDBAZodiac Zodiac : Zodiacs)
	{
		FDBAZodiacCreateListItem& Item = ZodiacItems.AddDefaulted_GetRef();
		Item.Zodiac = Zodiac;
		Item.DisplayName = StaticEnum<EDBAZodiac>()->GetDisplayNameTextByValue(static_cast<int64>(Zodiac));
	}
	BroadcastChanged();
}

void UDBACharacterCreateZodiacViewModel::ApplyDraft(const FDBACharacterCreateDraft& Draft)
{
	// Draft 是唯一创建业务状态；ViewModel 只复制可显示字段，不保存未创建角色的权威身份。
	SelectedZodiac = Draft.ZodiacType;
	Appearance = Draft.Appearance;
	for (FDBAZodiacCreateListItem& Item : ZodiacItems)
	{
		Item.bIsSelected = Item.Zodiac == SelectedZodiac;
	}
	BroadcastChanged();
}

void UDBACharacterCreateZodiacViewModel::ApplySelectedZodiacData(const UDBAZodiacHeroDataAsset& ZodiacData)
{
	// DataAsset 是名称、定位、难度、简介和头像的唯一静态来源；没有配置的难度保持为空。
	for (FDBAZodiacCreateListItem& Item : ZodiacItems)
	{
		if (Item.Zodiac != ZodiacData.ZodiacType) continue;
		Item.DisplayName = ZodiacData.DisplayName;
		Item.RoleSummary = ZodiacData.CharacterCreateRoleSummary;
		Item.Difficulty = ZodiacData.CharacterCreateDifficulty;
		Item.Description = ZodiacData.Description;
		Item.Portrait = ZodiacData.Portrait;
		break;
	}
	if (ZodiacData.ZodiacType == SelectedZodiac)
	{
		SelectedRoleSummary = ZodiacData.CharacterCreateRoleSummary;
		SelectedDifficulty = ZodiacData.CharacterCreateDifficulty;
		SelectedDescription = ZodiacData.Description;
	}
	BroadcastChanged();
}

void UDBACharacterCreateZodiacViewModel::ApplyAppearanceGroups(const TArray<FDBAAppearanceOptionGroup>& Groups)
{
	AppearanceGroups = Groups;
	BroadcastChanged();
}

void UDBACharacterCreateZodiacViewModel::SetPreviewLoading(const bool bLoading)
{
	if (bPreviewLoading == bLoading) return;
	bPreviewLoading = bLoading;
	BroadcastChanged();
}

void UDBACharacterCreateZodiacViewModel::SetValidationMessage(const FText& Message)
{
	ValidationMessage = Message;
	BroadcastChanged();
}

void UDBACharacterCreateZodiacViewModel::BroadcastChanged()
{
	OnChanged.Broadcast();
}
