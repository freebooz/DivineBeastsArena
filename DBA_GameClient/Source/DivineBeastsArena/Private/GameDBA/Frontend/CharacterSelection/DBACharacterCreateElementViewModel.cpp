// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementViewModel.h"

#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"

void UDBACharacterCreateElementViewModel::ApplyDraft(const FDBACharacterCreateDraft& Draft)
{
	// 只同步草稿中的元素选中态；技能组内容必须由 Controller 再次查询固定规则，
	// 不能信任 Draft 中任何可被本地恢复数据影响的展示缓存。
	SelectedElement = Draft.ElementType;
	for (FDBACharacterCreateElementCardModel& Card : ElementCards)
	{
		Card.bIsSelected = Card.Element == SelectedElement;
	}
	BroadcastChanged();
}

void UDBACharacterCreateElementViewModel::ApplyElementCards(const TArray<FDBACharacterCreateElementCardModel>& InCards)
{
	ElementCards = InCards;
	for (FDBACharacterCreateElementCardModel& Card : ElementCards)
	{
		Card.bIsSelected = Card.Element == SelectedElement;
	}
	BroadcastChanged();
}

void UDBACharacterCreateElementViewModel::ApplyFixedSkillBuild(const FDBAZodiacElementFixedSkillGroupRow* SkillGroup)
{
	FixedSkillBuildPreview = FDBAFixedSkillBuildPreviewModel();
	AttributePreview = FDBACharacterCreateAttributePreviewModel();
	if (!SkillGroup)
	{
		AttributePreview.Summary = NSLOCTEXT("DBACharacterCreateElement", "BuildLoading", "固定构筑配置加载中。");
		BroadcastChanged();
		return;
	}

	// 此处仅将策划表中的稳定技能 ID 依顺序展示，绝不暴露添加、移除或替换技能的接口。
	FixedSkillBuildPreview.FixedSkillBuildRowId = SkillGroup->RowId;
	FixedSkillBuildPreview.DisplayName = SkillGroup->DisplayName;
	FixedSkillBuildPreview.Description = SkillGroup->Description;
	FixedSkillBuildPreview.SkillIds = {
		SkillGroup->ElementPassiveSkillId,
		SkillGroup->ElementSkill1Id,
		SkillGroup->ElementSkill2Id,
		SkillGroup->ElementSkill3Id,
		SkillGroup->ElementSkill4Id,
		SkillGroup->ZodiacUltimateSkillId
	};
	FixedSkillBuildPreview.SkillIds.RemoveAll([](const FName SkillId) { return SkillId.IsNone(); });
	FixedSkillBuildPreview.ResonanceLevel = SkillGroup->ElementResonanceLevel;
	FixedSkillBuildPreview.ResonanceElement = SkillGroup->ResonanceElement;
	FixedSkillBuildPreview.bIsReady = SkillGroup->bEnabled && !SkillGroup->bIsInDevelopment && !SkillGroup->RowId.IsNone();

	AttributePreview.ResonanceControlTimeBonus = SkillGroup->ResonanceControlTimeBonus;
	AttributePreview.ResonanceShieldBonus = SkillGroup->ResonanceShieldBonus;
	AttributePreview.Summary = FText::Format(
		NSLOCTEXT("DBACharacterCreateElement", "AttributeSummary", "共鸣等级 {0}：控制时长 +{1}，护盾 +{2}%"),
		FText::AsNumber(SkillGroup->ElementResonanceLevel),
		FText::AsNumber(SkillGroup->ResonanceControlTimeBonus),
		FText::AsNumber(SkillGroup->ResonanceShieldBonus));
	BroadcastChanged();
}

void UDBACharacterCreateElementViewModel::SetValidationMessage(const FText& Message)
{
	ValidationMessage = Message;
	BroadcastChanged();
}

void UDBACharacterCreateElementViewModel::BroadcastChanged()
{
	OnChanged.Broadcast();
}
