// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetController.h"
#include "UDBAHeroSelectWidgetController.generated.h"

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAHeroSelectWidgetController : public UDBAWidgetController
{
	GENERATED_BODY()

public:
	UDBAHeroSelectWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|HeroSelect")
	void ConfirmZodiacSelection(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|HeroSelect")
	void RequestBack();

protected:
	UFUNCTION()
	void HandleZodiacConfirmed(bool bSuccess, EDBAZodiac Zodiac);

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZodiacConfirmed, EDBAZodiac, Zodiac);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|HeroSelect")
	FOnZodiacConfirmed OnZodiacConfirmed;
};
