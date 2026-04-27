// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "DBAPlayerUnitFrameWidgetBase.generated.h"

class UDBAPlayerUnitFrameWidgetController;

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAPlayerUnitFrameWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAPlayerUnitFrameWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
	virtual void NativeOnActivated();
	virtual void NativeOnDeactivated();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void SetWidgetController(UDBAPlayerUnitFrameWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void UpdateHP(float CurrentHP, float MaxHP);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void UpdateEnergy(float CurrentEnergy, float MaxEnergy);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void UpdateLevel(int32 Level);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PlayerUnitFrame")
	void ApplyFiveCampTheme(uint8 FiveCamp);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PlayerUnitFrame", meta = (DisplayName = "On Update HP"))
	void BP_OnUpdateHP(float CurrentHP, float MaxHP, float Percentage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PlayerUnitFrame", meta = (DisplayName = "On Update Energy"))
	void BP_OnUpdateEnergy(float CurrentEnergy, float MaxEnergy, float Percentage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PlayerUnitFrame", meta = (DisplayName = "On Update Level"))
	void BP_OnUpdateLevel(int32 Level);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PlayerUnitFrame", meta = (DisplayName = "On Apply FiveCamp Theme"))
	void BP_OnApplyFiveCampTheme(uint8 FiveCamp);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	TObjectPtr<UDBAPlayerUnitFrameWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedCurrentHP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedMaxHP;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedCurrentEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	float CachedMaxEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|PlayerUnitFrame")
	int32 CurrentLevel;
};
