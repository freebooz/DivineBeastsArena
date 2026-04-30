// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UUDBADebuffBarWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBADebuffBarWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBADebuffBarWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|DebuffBar")
	void AddDebuff(const FString& DebuffId, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|DebuffBar")
	void RemoveDebuff(const FString& DebuffId);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|DebuffBar")
	void ClearAllDebuffs();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|DebuffBar", meta = (DisplayName = "On Debuff Added"))
	void BP_OnDebuffAdded(const FString& DebuffId, float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|DebuffBar", meta = (DisplayName = "On Debuff Removed"))
	void BP_OnDebuffRemoved(const FString& DebuffId);
};
