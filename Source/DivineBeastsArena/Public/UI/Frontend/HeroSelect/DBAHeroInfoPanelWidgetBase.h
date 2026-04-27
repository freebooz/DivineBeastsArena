// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetBase.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBAHeroInfoPanelWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAHeroInfoPanelWidgetBase : public UDBAWidgetBase
{
	GENERATED_BODY()

public:
	UDBAHeroInfoPanelWidgetBase(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|HeroInfo")
	void SetZodiac(EDBAZodiac Zodiac);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|HeroInfo", meta = (DisplayName = "On Update Zodiac Info"))
	void BP_OnUpdateZodiacInfo(EDBAZodiac Zodiac, const FText& ZodiacName, const FText& ZodiacDescription, const FText& UltimateDescription);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|HeroInfo")
	EDBAZodiac CurrentZodiac;
};
