// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Widgets/Common/DBANetworkStatusWidgetBase.h"

void UDBANetworkStatusWidgetBase::SetNetworkStatus(bool bInAvailable, const FText& InStatusText)
{
	bIsNetworkAvailable = bInAvailable;
	StatusText = InStatusText;
	BP_OnNetworkStatusChanged(bIsNetworkAvailable, StatusText);
}
