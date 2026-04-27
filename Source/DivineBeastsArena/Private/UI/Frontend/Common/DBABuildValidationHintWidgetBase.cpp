// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Frontend/Common/DBABuildValidationHintWidgetBase.h"

UDBABuildValidationHintWidgetBase::UDBABuildValidationHintWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBABuildValidationHintWidgetBase::ShowValidationResult(bool bIsValid, const FText& ValidationMessage)
{
	BP_OnUpdateValidationResult(bIsValid, ValidationMessage);
}
