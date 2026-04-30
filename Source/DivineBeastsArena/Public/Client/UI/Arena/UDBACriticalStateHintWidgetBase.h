// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBACriticalStateHintWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBACriticalStateHintWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBACriticalStateHintWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|CriticalStateHint")
	void ShowCriticalHP(bool bLowHP);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|CriticalStateHint")
	void ShowCriticalEnergy(bool bLowEnergy);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|CriticalStateHint")
	void HideAllHints();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|CriticalStateHint", meta = (DisplayName = "On Critical State Changed"))
	void BP_OnCriticalStateChanged(bool bLowHP, bool bLowEnergy);
};
