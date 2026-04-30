// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "DBAAbilityBarWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAAbilityBarWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAAbilityBarWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|AbilityBar")
	void UpdateAbility(int32 SlotIndex, float Cooldown, float ManaCost);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|AbilityBar")
	void SetAbilityEnabled(int32 SlotIndex, bool bEnabled);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|AbilityBar", meta = (DisplayName = "On Ability Updated"))
	void BP_OnAbilityUpdated(int32 SlotIndex, float Cooldown, float ManaCost, bool bOnCooldown);
};
