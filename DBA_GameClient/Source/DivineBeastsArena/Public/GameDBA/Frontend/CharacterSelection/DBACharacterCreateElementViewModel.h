// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Data/Tables/DBAFixedSkillGroupData.h"
#include "DBACharacterCreateElementViewModel.generated.h"

struct FDBACharacterCreateDraft;

/** 单张元素卡片的显示投影；可用性完全来自固定技能组配置。 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBACharacterCreateElementCardModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Element")
	EDBAElement Element = EDBAElement::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Element")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Element")
	FText Description;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Element")
	bool bIsAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Element")
	bool bIsSelected = false;
};

/** 仅用于展示的固定技能组摘要；没有可编辑槽位，不能作为 GAS 授予来源。 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAFixedSkillBuildPreviewModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FixedBuild")
	FName FixedSkillBuildRowId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FixedBuild")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FixedBuild")
	FText Description;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FixedBuild")
	TArray<FName> SkillIds;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FixedBuild")
	int32 ResonanceLevel = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FixedBuild")
	EDBAElement ResonanceElement = EDBAElement::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FixedBuild")
	bool bIsReady = false;
};

/** 来自固定技能组共鸣字段的属性展示；不包含任何客户端战斗结算。 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBACharacterCreateAttributePreviewModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|AttributePreview")
	float ResonanceControlTimeBonus = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|AttributePreview")
	float ResonanceShieldBonus = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|AttributePreview")
	FText Summary;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnCharacterCreateElementViewModelChanged);

/**
 * 角色创建第二步的只读显示状态。Controller 将 Draft 和固定技能组规则投影到这里，
 * Widget 只订阅它绘制元素、技能与属性，不可反向组装自由 SkillLoadout。
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBACharacterCreateElementViewModel final : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Element")
	const TArray<FDBACharacterCreateElementCardModel>& GetElementCards() const { return ElementCards; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Element")
	EDBAElement GetSelectedElement() const { return SelectedElement; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|FixedBuild")
	const FDBAFixedSkillBuildPreviewModel& GetFixedSkillBuildPreview() const { return FixedSkillBuildPreview; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|AttributePreview")
	const FDBACharacterCreateAttributePreviewModel& GetAttributePreview() const { return AttributePreview; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Element")
	const FText& GetValidationMessage() const { return ValidationMessage; }

	void ApplyDraft(const FDBACharacterCreateDraft& Draft);
	void ApplyElementCards(const TArray<FDBACharacterCreateElementCardModel>& InCards);
	void ApplyFixedSkillBuild(const FDBAZodiacElementFixedSkillGroupRow* SkillGroup);
	void SetValidationMessage(const FText& Message);

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterCreate|Element")
	FDBAOnCharacterCreateElementViewModelChanged OnChanged;

private:
	void BroadcastChanged();

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate|Element")
	TArray<FDBACharacterCreateElementCardModel> ElementCards;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate|Element")
	EDBAElement SelectedElement = EDBAElement::None;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate|FixedBuild")
	FDBAFixedSkillBuildPreviewModel FixedSkillBuildPreview;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate|AttributePreview")
	FDBACharacterCreateAttributePreviewModel AttributePreview;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate|Element")
	FText ValidationMessage;
};
