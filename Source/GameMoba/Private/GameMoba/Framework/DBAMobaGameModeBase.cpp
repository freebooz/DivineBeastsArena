// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameMoba/Framework/DBAMobaGameModeBase.h"

#include "Engine/World.h"

ADBAMobaGameModeBase::ADBAMobaGameModeBase()
{
}

void ADBAMobaGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ADBAMobaGameModeBase::InitGameState()
{
	Super::InitGameState();
}

void ADBAMobaGameModeBase::StartMatch()
{
	bMatchStarted = true;
	MatchStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void ADBAMobaGameModeBase::HandleMatchHasStarted()
{
	bMatchStarted = true;
}

void ADBAMobaGameModeBase::HandleMatchHasEnded()
{
	bMatchEnded = true;
	MatchEndTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}
