// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DBABootPlayerController.generated.h"

/** Input-free controller for L_DBA_Boot. UI is only created after travel to the persistent frontend map. */
UCLASS()
class DIVINEBEASTSARENA_API ADBABootPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
