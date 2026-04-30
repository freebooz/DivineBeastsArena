// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBABuffBarWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBABuffBarWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBABuffBarWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|BuffBar")
	void AddBuff(const FString& BuffId, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|BuffBar")
	void RemoveBuff(const FString& BuffId);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|BuffBar")
	void ClearAllBuffs();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|BuffBar", meta = (DisplayName = "On Buff Added"))
	void BP_OnBuffAdded(const FString& BuffId, float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|BuffBar", meta = (DisplayName = "On Buff Removed"))
	void BP_OnBuffRemoved(const FString& BuffId);
};
