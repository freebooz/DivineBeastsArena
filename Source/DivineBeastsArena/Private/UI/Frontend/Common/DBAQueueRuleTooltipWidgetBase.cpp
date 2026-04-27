// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Frontend/Common/DBAQueueRuleTooltipWidgetBase.h"

UDBAQueueRuleTooltipWidgetBase::UDBAQueueRuleTooltipWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAQueueRuleTooltipWidgetBase::ShowQueueRule(
	const FText& QueueName,
	int32 TeamSize,
	const FText& MapName,
	const FText& ModeName,
	const FText& RuleDescription)
{
	BP_OnUpdateQueueRule(QueueName, TeamSize, MapName, ModeName, RuleDescription);
}
