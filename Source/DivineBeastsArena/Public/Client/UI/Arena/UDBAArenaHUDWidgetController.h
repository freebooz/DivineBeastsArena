// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MobaBase/UI/UDBAMobaHUDWidgetControllerBase.h"
#include "DBAArenaHUDWidgetController.generated.h"

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAArenaHUDWidgetController : public UDBAMobaHUDWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBAArenaHUDWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	void UpdatePlayerHP(float CurrentHP, float MaxHP) override;

	void UpdatePlayerEnergy(float CurrentEnergy, float MaxEnergy);

	void UpdateUltimateEnergy(float CurrentEnergy, float MaxEnergy);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerEnergyChanged, float, CurrentEnergy, float, MaxEnergy);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD")
	FOnPlayerEnergyChanged OnPlayerEnergyChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUltimateEnergyChanged, float, CurrentEnergy, float, MaxEnergy);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|ArenaHUD")
	FOnUltimateEnergyChanged OnUltimateEnergyChanged;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float CurrentHP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float MaxHP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float CurrentEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	float MaxEnergy;
};
