// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "DBALoadingScreenWidgetBase.generated.h"

class UDBALoadingWidgetController;

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBALoadingScreenWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBALoadingScreenWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeOnActivated();
	virtual void NativeOnDeactivated();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Loading")
	void SetWidgetController(UDBALoadingWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Loading")
	void UpdateLoadingProgress(float Progress);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Loading")
	void ShowTips(const FText& TipsText);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|Loading", meta = (DisplayName = "On Loading Progress Updated"))
	void BP_OnLoadingProgressUpdated(float Progress);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|Loading", meta = (DisplayName = "On Tips Updated"))
	void BP_OnTipsUpdated(const FText& TipsText);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Loading")
	TObjectPtr<UDBALoadingWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Loading")
	float CurrentProgress;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Loading")
	FText CurrentTips;
};
