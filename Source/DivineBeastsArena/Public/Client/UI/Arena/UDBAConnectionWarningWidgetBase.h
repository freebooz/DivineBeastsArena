// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UUDBAConnectionWarningWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAConnectionWarningWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAConnectionWarningWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ConnectionWarning")
	void ShowWarning(const FText& Message);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ConnectionWarning")
	void HideWarning();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ConnectionWarning", meta = (DisplayName = "On Warning Shown"))
	void BP_OnWarningShown(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ConnectionWarning", meta = (DisplayName = "On Warning Hidden"))
	void BP_OnWarningHidden();
};
