// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetController.h"
#include "DBAFiveCampSelectWidgetController.generated.h"

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAFiveCampSelectWidgetController : public UDBAWidgetController
{
	GENERATED_BODY()

public:
	UDBAFiveCampSelectWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void ConfirmFiveCampSelection(EDBAFiveCamp FiveCamp);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void RequestBack();

protected:
	UFUNCTION()
	void HandleFiveCampConfirmed(bool bSuccess, EDBAFiveCamp FiveCamp);

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFiveCampConfirmed, EDBAFiveCamp, FiveCamp);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|FiveCampSelect")
	FOnFiveCampConfirmed OnFiveCampConfirmed;
};
