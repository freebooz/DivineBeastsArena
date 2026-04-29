// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBAElementSelectWidgetBase.generated.h"

class UDBAElementInfoPanelWidgetBase;
class UDBAFixedSkillGroupPreviewWidgetBase;
class UDBAElementSelectWidgetController;

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAElementSelectWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAElementSelectWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	virtual void NativeOnActivated();
	virtual void NativeOnDeactivated();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void SetWidgetController(UDBAElementSelectWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void SetSelectedZodiac(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void RefreshElementList();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void SelectElement(EDBAElement Element);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void ConfirmElementSelection();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void OnBackButtonClicked();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ElementSelect", meta = (DisplayName = "On Refresh Element List"))
	void BP_OnRefreshElementList(const TArray<EDBAElement>& AvailableElements);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ElementSelect", meta = (DisplayName = "On Element Selected"))
	void BP_OnElementSelected(EDBAElement Element);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ElementSelect", meta = (DisplayName = "On Confirm Button State Changed"))
	void BP_OnConfirmButtonStateChanged(bool bCanConfirm);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect", meta = (BindWidget))
	TObjectPtr<UDBAElementInfoPanelWidgetBase> ElementInfoPanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect", meta = (BindWidget))
	TObjectPtr<UDBAFixedSkillGroupPreviewWidgetBase> SkillGroupPreview;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect")
	TObjectPtr<UDBAElementSelectWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect")
	EDBAZodiac SelectedZodiac;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect")
	EDBAElement CurrentSelectedElement;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect")
	bool bHasSelectedElement;
};
