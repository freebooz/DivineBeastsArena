// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Character/Appearance/DBACharacterAppearanceTypes.h"
#include "DBACharacterCreateZodiacViewModel.generated.h"

class UTexture2D;
class UDBAZodiacHeroDataAsset;
struct FDBACharacterCreateDraft;

/** ZodiacRegistry 元数据到创建页列表项的显示投影；不含 Actor、JSON 或网络 DTO。 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAZodiacCreateListItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Zodiac")
	EDBAZodiac Zodiac = EDBAZodiac::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Zodiac")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Zodiac")
	FText RoleSummary;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Zodiac")
	FText Difficulty;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Zodiac")
	FText Description;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Zodiac")
	TSoftObjectPtr<UTexture2D> Portrait;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Zodiac")
	bool bIsSelected = false;
};

/** 一个外观槽位及其由 Draft/Catalog 联合过滤后的可选稳定 ID。 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAAppearanceOptionGroup
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Appearance")
	EDBAAppearanceSlot Slot = EDBAAppearanceSlot::Hair;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Appearance")
	TArray<FName> OptionIds;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Appearance")
	FName SelectedOptionId = NAME_None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnCharacterCreateZodiacViewModelChanged);

/**
 * 创建第一步的纯显示状态。Controller 将 Draft、Registry 和 Preview 的事件投影到这里，
 * Widget 只订阅本对象，不直接访问 Subsystem 或 AssetManager。
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBACharacterCreateZodiacViewModel final : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Zodiac")
	const TArray<FDBAZodiacCreateListItem>& GetZodiacItems() const { return ZodiacItems; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Appearance")
	const TArray<FDBAAppearanceOptionGroup>& GetAppearanceGroups() const { return AppearanceGroups; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Zodiac")
	EDBAZodiac GetSelectedZodiac() const { return SelectedZodiac; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Appearance")
	const FDBACharacterAppearance& GetAppearance() const { return Appearance; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Preview")
	bool IsPreviewLoading() const { return bPreviewLoading; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Zodiac")
	const FText& GetSelectedRoleSummary() const { return SelectedRoleSummary; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Zodiac")
	const FText& GetSelectedDifficulty() const { return SelectedDifficulty; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Zodiac")
	const FText& GetSelectedDescription() const { return SelectedDescription; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|Zodiac")
	const FText& GetValidationMessage() const { return ValidationMessage; }

	void SetAvailableZodiacs(const TArray<EDBAZodiac>& Zodiacs);
	void ApplyDraft(const FDBACharacterCreateDraft& Draft);
	void ApplySelectedZodiacData(const UDBAZodiacHeroDataAsset& ZodiacData);
	void ApplyAppearanceGroups(const TArray<FDBAAppearanceOptionGroup>& Groups);
	void SetPreviewLoading(bool bLoading);
	void SetValidationMessage(const FText& Message);

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterCreate")
	FDBAOnCharacterCreateZodiacViewModelChanged OnChanged;

private:
	void BroadcastChanged();

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate")
	TArray<FDBAZodiacCreateListItem> ZodiacItems;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate")
	TArray<FDBAAppearanceOptionGroup> AppearanceGroups;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate")
	EDBAZodiac SelectedZodiac = EDBAZodiac::None;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate")
	FDBACharacterAppearance Appearance;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate")
	FText SelectedRoleSummary;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate")
	FText SelectedDifficulty;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate")
	FText SelectedDescription;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate")
	FText ValidationMessage;

	bool bPreviewLoading = false;
};
