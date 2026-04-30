// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/Common/UDBABuildValidationHintWidgetBase.h"

UDBABuildValidationHintWidgetBase::UDBABuildValidationHintWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBABuildValidationHintWidgetBase::ShowValidationResult(bool bIsValid, const FText& ValidationMessage)
{
	BP_OnUpdateValidationResult(bIsValid, ValidationMessage);
}
