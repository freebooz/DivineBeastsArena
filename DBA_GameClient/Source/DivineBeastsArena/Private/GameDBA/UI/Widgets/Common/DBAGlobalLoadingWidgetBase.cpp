// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Widgets/Common/DBAGlobalLoadingWidgetBase.h"

void UDBAGlobalLoadingWidgetBase::SetLoadingMessage(const FText& InMessage)
{
	LoadingMessage = InMessage;
	BP_OnLoadingMessageChanged(LoadingMessage);
}
