// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/HeroSelect/DBAHeroSelectWidgetController.h"

UDBAHeroSelectWidgetController::UDBAHeroSelectWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAHeroSelectWidgetController::ConfirmZodiacSelection(EDBAZodiac Zodiac)
{
	HandleZodiacConfirmed(true, Zodiac);
}

void UDBAHeroSelectWidgetController::RequestBack()
{
}

void UDBAHeroSelectWidgetController::HandleZodiacConfirmed(bool bSuccess, EDBAZodiac Zodiac)
{
	if (bSuccess)
	{
		OnZodiacConfirmed.Broadcast(Zodiac);
	}
}
