// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Widgets/Common/DBASystemToastWidgetBase.h"

void UDBASystemToastWidgetBase::ShowMessage(const FText& InMessage)
{
	Message = InMessage;
	BP_OnToastMessageChanged(Message);
}
