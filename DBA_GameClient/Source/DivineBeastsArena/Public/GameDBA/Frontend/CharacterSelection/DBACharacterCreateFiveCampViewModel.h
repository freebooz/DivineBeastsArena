// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Data/Tables/DBAFiveCampDisplayData.h"
#include "DBACharacterCreateFiveCampViewModel.generated.h"

struct FDBACharacterCreateDraft;
class UTexture2D;

/**
 * 单张五营卡片的只读显示投影。
 *
 * 数据来自 CharacterCreateFiveCampDisplayTable；Widget 不得根据 East/West 等枚举写死名称、颜色、
 * 图标或可用性。该投影不包含 TeamId 与 Faction GameplayTag，从类型层面隔离账号创建和对局阵营。
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBACharacterCreateFiveCampCardModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	EDBAFiveCamp FiveCamp = EDBAFiveCamp::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	FName SourceRowName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	FText Description;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	TSoftObjectPtr<UTexture2D> Emblem;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	FLinearColor ThemeColor = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	FLinearColor SecondaryColor = FLinearColor::Gray;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	bool bIsAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	bool bIsSelected = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	FText UnavailableReason;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnCharacterCreateFiveCampViewModelChanged);

/**
 * 角色创建第三步的只读 UI 状态。Controller 负责异步加载数据表、校验表行并应用 PreviewStage 主题；
 * 此 ViewModel 只保存可绑定投影，因此 UMG 不需要，也不能直接持有 DataTable、PreviewActor 或 Draft。
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBACharacterCreateFiveCampViewModel final : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|FiveCamp")
	const TArray<FDBACharacterCreateFiveCampCardModel>& GetFiveCampCards() const { return FiveCampCards; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|FiveCamp")
	EDBAFiveCamp GetSelectedFiveCamp() const { return SelectedFiveCamp; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|FiveCamp")
	const FDBACharacterCreateFiveCampCardModel& GetSelectedCard() const { return SelectedCard; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterCreate|FiveCamp")
	const FText& GetValidationMessage() const { return ValidationMessage; }

	void ApplyDraft(const FDBACharacterCreateDraft& Draft);
	void ApplyFiveCampCards(const TArray<FDBACharacterCreateFiveCampCardModel>& InCards);
	void SetValidationMessage(const FText& Message);

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterCreate|FiveCamp")
	FDBAOnCharacterCreateFiveCampViewModelChanged OnChanged;

private:
	void RefreshSelection();

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate|FiveCamp")
	TArray<FDBACharacterCreateFiveCampCardModel> FiveCampCards;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate|FiveCamp")
	EDBAFiveCamp SelectedFiveCamp = EDBAFiveCamp::None;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate|FiveCamp")
	FDBACharacterCreateFiveCampCardModel SelectedCard;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterCreate|FiveCamp")
	FText ValidationMessage;
};
