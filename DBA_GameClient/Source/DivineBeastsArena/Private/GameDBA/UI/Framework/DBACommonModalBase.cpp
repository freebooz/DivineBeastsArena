// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Framework/DBACommonModalBase.h"

UDBACommonModalBase::UDBACommonModalBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACommonModalBase::Dismiss()
{
	RequestBack();
}
