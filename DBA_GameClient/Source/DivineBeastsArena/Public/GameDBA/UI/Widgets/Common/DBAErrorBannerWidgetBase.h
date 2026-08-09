// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBAErrorBannerWidgetBase.generated.h"

/** Blueprint base for WBP_DBA_ErrorBanner. It displays a localized, mapped business error. */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAErrorBannerWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="DBA|UI|Error")
	void ShowError(const FText& InMessage);

	UPROPERTY(BlueprintReadOnly, Category="DBA|UI|Error")
	FText ErrorMessage;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="DBA|UI|Error", meta=(DisplayName="On Error Changed"))
	void BP_OnErrorChanged(const FText& InMessage);
};
