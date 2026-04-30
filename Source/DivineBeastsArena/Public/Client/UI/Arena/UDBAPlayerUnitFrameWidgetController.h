// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetController.h"
#include "UDBAPlayerUnitFrameWidgetController.generated.h"

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAPlayerUnitFrameWidgetController : public UDBAWidgetController
{
	GENERATED_BODY()

public:
	UDBAPlayerUnitFrameWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	float GetCurrentHP() const;

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	float GetMaxHP() const;

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	float GetCurrentEnergy() const;

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	float GetMaxEnergy() const;

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	int32 GetCurrentLevel() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPUpdated, float, CurrentHP, float, MaxHP);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|PlayerUnitFrame")
	FOnHPUpdated OnHPUpdated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnergyUpdated, float, CurrentEnergy, float, MaxEnergy);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|PlayerUnitFrame")
	FOnEnergyUpdated OnEnergyUpdated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUpdated, int32, Level);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|PlayerUnitFrame")
	FOnLevelUpdated OnLevelUpdated;
};
