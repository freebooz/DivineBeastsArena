// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DBABootGameMode.generated.h"

/** Minimal client boot mode. It creates no pawn, character presentation, UI, VFX, or gameplay state. */
UCLASS()
class DIVINEBEASTSARENA_API ADBABootGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADBABootGameMode();
};
