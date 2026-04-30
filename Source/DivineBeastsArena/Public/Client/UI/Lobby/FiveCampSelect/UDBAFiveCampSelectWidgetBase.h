// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBAFiveCampSelectWidgetBase.generated.h"

class UDBAFiveCampInfoPanelWidgetBase;
class UDBAFiveCampSelectWidgetController;

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAFiveCampSelectWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAFiveCampSelectWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	virtual void NativeOnActivated();
	virtual void NativeOnDeactivated();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void SetWidgetController(UDBAFiveCampSelectWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void SetSelectedZodiacAndElement(EDBAZodiac Zodiac, EDBAElement Element);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void RefreshFiveCampList();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void SelectFiveCamp(EDBAFiveCamp FiveCamp);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void ConfirmFiveCampSelection();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void OnBackButtonClicked();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|FiveCampSelect", meta = (DisplayName = "On Refresh FiveCamp List"))
	void BP_OnRefreshFiveCampList(const TArray<EDBAFiveCamp>& AvailableFiveCamps);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|FiveCampSelect", meta = (DisplayName = "On FiveCamp Selected"))
	void BP_OnFiveCampSelected(EDBAFiveCamp FiveCamp);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|FiveCampSelect", meta = (DisplayName = "On Confirm Button State Changed"))
	void BP_OnConfirmButtonStateChanged(bool bCanConfirm);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect", meta = (BindWidget))
	TObjectPtr<UDBAFiveCampInfoPanelWidgetBase> FiveCampInfoPanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect")
	TObjectPtr<UDBAFiveCampSelectWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect")
	EDBAZodiac SelectedZodiac;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect")
	EDBAElement SelectedElement;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect")
	EDBAFiveCamp CurrentSelectedFiveCamp;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect")
	bool bHasSelectedFiveCamp;
};
