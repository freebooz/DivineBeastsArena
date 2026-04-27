// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBAAuraSummaryPanelWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAAuraSummaryPanelWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAAuraSummaryPanelWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|AuraSummaryPanel")
	void UpdateAuraCount(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|AuraSummaryPanel")
	void ShowAuraDetails();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|AuraSummaryPanel", meta = (DisplayName = "On Aura Count Updated"))
	void BP_OnAuraCountUpdated(int32 Count);
};
