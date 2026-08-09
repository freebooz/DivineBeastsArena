// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Startup/DBABootPlayerController.h"

void ADBABootPlayerController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = false;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
}
