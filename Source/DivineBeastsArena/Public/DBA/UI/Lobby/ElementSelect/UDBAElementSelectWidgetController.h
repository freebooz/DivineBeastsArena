// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetController.h"
#include "UDBAElementSelectWidgetController.generated.h"

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAElementSelectWidgetController : public UDBAWidgetController
{
	GENERATED_BODY()

public:
	UDBAElementSelectWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void ConfirmElementSelection(EDBAElement Element);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void RequestBack();

protected:
	UFUNCTION()
	void HandleElementConfirmed(bool bSuccess, EDBAElement Element);

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnElementConfirmed, EDBAElement, Element);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ElementSelect")
	FOnElementConfirmed OnElementConfirmed;
};
