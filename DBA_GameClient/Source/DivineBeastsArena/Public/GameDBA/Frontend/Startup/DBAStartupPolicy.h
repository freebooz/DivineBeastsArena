// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "DBAStartupPolicy.generated.h"

UENUM()
enum class EDBAStartupCheckDisposition : uint8
{
	TravelToFrontend,
	FatalFailure
};

namespace DBAStartupPolicy
{
	DIVINEBEASTSARENA_API bool IsFrontendMapConfigurationValid(const FSoftObjectPath& MapPath);
	DIVINEBEASTSARENA_API EDBAStartupCheckDisposition ResolveBackendCheck(bool bConfigurationValid, bool bBackendReachable);
}
