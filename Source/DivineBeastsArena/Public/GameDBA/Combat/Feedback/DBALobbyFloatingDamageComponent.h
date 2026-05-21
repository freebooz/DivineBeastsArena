// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Combat/Feedback/DBAFloatingDamageComponent.h"
#include "DBALobbyFloatingDamageComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "DBA Lobby Floating Damage Component"))
class DIVINEBEASTSARENA_API UDBALobbyFloatingDamageComponent : public UDBAFloatingDamageComponent
{
	GENERATED_BODY()

public:
	UDBALobbyFloatingDamageComponent();
};
