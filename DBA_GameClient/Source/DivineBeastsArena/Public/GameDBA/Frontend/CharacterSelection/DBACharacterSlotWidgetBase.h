// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameCore/Networking/Account/DBAAccountTypes.h"
#include "DBACharacterSlotWidgetBase.generated.h"

class UTextBlock;
class UImage;

/** WBP_DBA_CharacterSlot 的轻量 C++ 父类；只表现领域摘要，不访问网络或 Flow。 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBACharacterSlotWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void SetCharacterSummary(const FDBACharacterSummary& InSummary, bool bInSelected);

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	const FDBACharacterSummary& GetCharacterSummary() const { return CharacterSummary; }

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterSelect")
	bool IsSelected() const { return bSelected; }

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect") TObjectPtr<UTextBlock> NameText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect") TObjectPtr<UTextBlock> ZodiacText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect") TObjectPtr<UTextBlock> LevelText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect") TObjectPtr<UTextBlock> LastPlayedText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect") TObjectPtr<UTextBlock> LocationText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect") TObjectPtr<UImage> PortraitImage;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterSelect") FDBACharacterSummary CharacterSummary;
	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterSelect") bool bSelected = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterSelect")
	void BP_OnSlotChanged(const FDBACharacterSummary& Summary, bool bInSelected);
};
