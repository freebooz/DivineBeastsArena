// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UUDBASelfCastBarWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBASelfCastBarWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBASelfCastBarWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|SelfCastBar")
	void ShowSelfCastProgress(float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|SelfCastBar")
	void HideSelfCastProgress();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|SelfCastBar", meta = (DisplayName = "On Self Cast Progress"))
	void BP_OnSelfCastProgress(float Duration, float RemainingTime);
};
