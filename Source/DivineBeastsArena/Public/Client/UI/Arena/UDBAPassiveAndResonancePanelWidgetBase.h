// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "DBAPassiveAndResonancePanelWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAPassiveAndResonancePanelWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAPassiveAndResonancePanelWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PassivePanel")
	void UpdatePassiveSkill(int32 SlotIndex, bool bActive);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|PassivePanel")
	void UpdateResonanceLevel(int32 Level);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PassivePanel", meta = (DisplayName = "On Passive Updated"))
	void BP_OnPassiveUpdated(int32 SlotIndex, bool bActive);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|PassivePanel", meta = (DisplayName = "On Resonance Level Updated"))
	void BP_OnResonanceLevelUpdated(int32 Level);
};
