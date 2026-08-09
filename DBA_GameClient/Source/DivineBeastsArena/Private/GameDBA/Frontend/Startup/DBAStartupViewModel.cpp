// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Startup/DBAStartupViewModel.h"

void UDBAStartupViewModel::SetPresentation(const FText& InTitle, const FText& InVersionText)
{
	Title = InTitle;
	VersionText = InVersionText;
	OnChanged.Broadcast();
}

void UDBAStartupViewModel::SetServiceStatus(EDBAStartupServiceState InState, const FText& InStatusText, bool bInCanContinue)
{
	ServiceState = InState;
	ServiceStatusText = InStatusText;
	bCanContinue = bInCanContinue;
	OnChanged.Broadcast();
}
