#pragma once

#include "GameCore/Data/DBADataAssetBase.h"
#include "DBADataAsset.generated.h"

/**
 * Legacy compatibility data asset class.
 * Keeps old native class path /Script/DivineBeastsArena.DBADataAsset loadable.
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBADataAsset : public UDBADataAssetBase
{
	GENERATED_BODY()
};

