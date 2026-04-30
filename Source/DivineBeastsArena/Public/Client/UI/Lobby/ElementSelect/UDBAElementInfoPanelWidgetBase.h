// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetBase.h"
#include "UDBAElementInfoPanelWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAElementInfoPanelWidgetBase : public UDBAWidgetBase
{
	GENERATED_BODY()

public:
	UDBAElementInfoPanelWidgetBase(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementInfo")
	void SetElement(EDBAElement Element);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ElementInfo", meta = (DisplayName = "On Update Element Info"))
	void BP_OnUpdateElementInfo(EDBAElement Element, const FText& ElementName, const FText& ElementDescription, EDBAElement CounterTo, EDBAElement CounteredBy);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementInfo")
	EDBAElement CurrentElement;
};
