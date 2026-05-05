// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetBase.h"
#include "UDBAFiveCampInfoPanelWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAFiveCampInfoPanelWidgetBase : public UDBAWidgetBase
{
	GENERATED_BODY()

public:
	UDBAFiveCampInfoPanelWidgetBase(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampInfo")
	void SetFiveCamp(EDBAFiveCamp FiveCamp, EDBAZodiac Zodiac, EDBAElement Element);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|FiveCampInfo", meta = (DisplayName = "On Update FiveCamp Info"))
	void BP_OnUpdateFiveCampInfo(
		EDBAFiveCamp FiveCamp,
		const FText& FiveCampName,
		const FText& FiveCampDescription,
		const FText& AppearanceTheme,
		const FText& EffectTheme
	);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampInfo")
	EDBAFiveCamp CurrentFiveCamp;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampInfo")
	EDBAZodiac CurrentZodiac;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampInfo")
	EDBAElement CurrentElement;
};
