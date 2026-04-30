// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "DBAHeroSelectWidgetBase.generated.h"

class UDBAHeroInfoPanelWidgetBase;
class UDBAHeroSelectWidgetController;

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAHeroSelectWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAHeroSelectWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnActivated();
	virtual void NativeOnDeactivated();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|HeroSelect")
	void SetWidgetController(UDBAHeroSelectWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|HeroSelect")
	void RefreshZodiacList();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|HeroSelect")
	void SelectZodiac(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|HeroSelect")
	void ConfirmZodiacSelection();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|HeroSelect")
	void OnBackButtonClicked();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|HeroSelect", meta = (DisplayName = "On Refresh Zodiac List"))
	void BP_OnRefreshZodiacList(const TArray<EDBAZodiac>& AvailableZodiacs);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|HeroSelect", meta = (DisplayName = "On Zodiac Selected"))
	void BP_OnZodiacSelected(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|HeroSelect", meta = (DisplayName = "On Confirm Button State Changed"))
	void BP_OnConfirmButtonStateChanged(bool bCanConfirm);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|HeroSelect", meta = (BindWidget))
	TObjectPtr<UDBAHeroInfoPanelWidgetBase> HeroInfoPanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|HeroSelect")
	TObjectPtr<UDBAHeroSelectWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|HeroSelect")
	EDBAZodiac CurrentSelectedZodiac;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|HeroSelect")
	bool bHasSelectedZodiac;
};
