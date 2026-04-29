// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetBase.h"
#include "DBABuildValidationHintWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBABuildValidationHintWidgetBase : public UDBAWidgetBase
{
	GENERATED_BODY()

public:
	UDBABuildValidationHintWidgetBase(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|BuildValidation")
	void ShowValidationResult(bool bIsValid, const FText& ValidationMessage);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|BuildValidation", meta = (DisplayName = "On Update Validation Result"))
	void BP_OnUpdateValidationResult(bool bIsValid, const FText& ValidationMessage);
};
