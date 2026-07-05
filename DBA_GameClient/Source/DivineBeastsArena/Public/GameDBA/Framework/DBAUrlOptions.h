// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Character/DBACharacterBuildTypes.h"

namespace DBAUrlOptions
{
	DIVINEBEASTSARENA_API FString ExtractUrlOption(const FString& Options, const FString& Key);
	DIVINEBEASTSARENA_API bool TryExtractCharacterBuildSummary(const FString& Options, FDBACharacterBuildSummary& OutSummary);
	DIVINEBEASTSARENA_API bool TryExtractTeamId(const FString& Options, int32& OutTeamId);
}
