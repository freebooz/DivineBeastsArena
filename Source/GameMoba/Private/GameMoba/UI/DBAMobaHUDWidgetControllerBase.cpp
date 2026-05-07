// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameMoba/UI/DBAMobaHUDWidgetControllerBase.h"

#include "GameFramework/PlayerController.h"

UDBAMobaHUDWidgetControllerBase::UDBAMobaHUDWidgetControllerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAMobaHUDWidgetControllerBase::InitializeController(APlayerController* InPlayerController)
{
	PlayerController = InPlayerController;
	bIsInitialized = InPlayerController != nullptr;
}

void UDBAMobaHUDWidgetControllerBase::UpdatePlayerHP(float CurrentHP, float MaxHP)
{
}
