// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBAMomentumPanelWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAMomentumPanelWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAMomentumPanelWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|MomentumPanel")
	void UpdateMomentumLevel(int32 Level);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|MomentumPanel")
	void UpdateMomentumProgress(float Progress);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|MomentumPanel", meta = (DisplayName = "On Momentum Updated"))
	void BP_OnMomentumUpdated(int32 Level, float Progress);
};
