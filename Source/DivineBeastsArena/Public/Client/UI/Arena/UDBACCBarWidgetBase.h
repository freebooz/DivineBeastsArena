// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBACCBarWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBACCBarWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBACCBarWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|CCBar")
	void AddCCEffect(const FString& CCId, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|CCBar")
	void RemoveCCEffect(const FString& CCId);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|CCBar")
	void ClearAllCCEffects();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|CCBar", meta = (DisplayName = "On CC Effect Added"))
	void BP_OnCCEffectAdded(const FString& CCId, float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|CCBar", meta = (DisplayName = "On CC Effect Removed"))
	void BP_OnCCEffectRemoved(const FString& CCId);
};
