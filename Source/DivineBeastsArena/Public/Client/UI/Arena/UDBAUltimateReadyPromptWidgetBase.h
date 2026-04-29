// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBAUltimateReadyPromptWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAUltimateReadyPromptWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAUltimateReadyPromptWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|UltimateReadyPrompt")
	void ShowUltimateReady();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|UltimateReadyPrompt")
	void HideUltimateReady();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|UltimateReadyPrompt", meta = (DisplayName = "On Ultimate Ready"))
	void BP_OnUltimateReady();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|UltimateReadyPrompt", meta = (DisplayName = "On Ultimate Hidden"))
	void BP_OnUltimateHidden();
};
