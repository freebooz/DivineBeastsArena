// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBAQueueRuleTooltipWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAQueueRuleTooltipWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAQueueRuleTooltipWidgetBase(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|QueueRule")
	void ShowQueueRule(
		const FText& QueueName,
		int32 TeamSize,
		const FText& MapName,
		const FText& ModeName,
		const FText& RuleDescription
	);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|QueueRule", meta = (DisplayName = "On Update Queue Rule"))
	void BP_OnUpdateQueueRule(
		const FText& QueueName,
		int32 TeamSize,
		const FText& MapName,
		const FText& ModeName,
		const FText& RuleDescription
	);
};
