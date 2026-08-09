// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Framework/GameModes/DBABootGameMode.h"

#include "GameDBA/Frontend/Startup/DBABootPlayerController.h"

ADBABootGameMode::ADBABootGameMode()
{
	PlayerControllerClass = ADBABootPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	bStartPlayersAsSpectators = true;
}
