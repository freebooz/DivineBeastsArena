// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameDBA/Frontend/Character/DBACharacterRosterSubsystem.h"
#include "DBACharacterInfoPanelWidgetBase.generated.h"

class UTextBlock;

/** WBP_DBA_CharacterInfoPanel 的 C++ 父类，展示角色详情及独立预览加载状态。 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBACharacterInfoPanelWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void SetCharacterDetails(const FDBACharacterDetails& InDetails, bool bPreviewLoading);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect") TObjectPtr<UTextBlock> NameText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect") TObjectPtr<UTextBlock> ZodiacText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect") TObjectPtr<UTextBlock> LevelText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect") TObjectPtr<UTextBlock> PreviewStatusText;
	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterSelect") FDBACharacterDetails CharacterDetails;
	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterSelect") bool bIsPreviewLoading = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterSelect")
	void BP_OnCharacterDetailsChanged(const FDBACharacterDetails& Details, bool bInPreviewLoading);
};
