// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Widgets/Common/DBAErrorBannerWidgetBase.h"

void UDBAErrorBannerWidgetBase::ShowError(const FText& InMessage)
{
	ErrorMessage = InMessage;
	BP_OnErrorChanged(ErrorMessage);
}
