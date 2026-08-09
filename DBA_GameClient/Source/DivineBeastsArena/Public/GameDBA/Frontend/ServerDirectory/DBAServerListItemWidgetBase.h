// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameDBA/Frontend/ServerDirectory/DBAServerSelectViewModel.h"
#include "DBAServerListItemWidgetBase.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnServerListItemChosen, const FString&, ServerId);

/** WBP_DBA_ServerListItem 的 C++ 父类；仅显示 ViewModel 数据并上报用户意图。 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAServerListItemWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|ServerSelect")
	void SetViewData(const FDBAServerSelectItemViewData& InViewData);

	UPROPERTY(BlueprintAssignable, Category = "DBA|ServerSelect")
	FDBAOnServerListItemChosen OnChosen;

protected:
	virtual void NativeOnInitialized() override;
	void EnsureFallbackLayout();
	void RefreshPresentation();

	UFUNCTION()
	void HandleClicked();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|ServerSelect")
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|ServerSelect")
	TObjectPtr<UTextBlock> SummaryText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ServerSelect")
	FDBAServerSelectItemViewData ViewData;
};
