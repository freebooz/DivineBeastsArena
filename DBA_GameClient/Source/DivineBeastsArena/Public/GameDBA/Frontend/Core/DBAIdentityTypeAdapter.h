// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Core/DBAEnumsCore.h"

/**
 * 旧项目身份枚举到前台规范枚举的唯一兼容边界。
 * 新前台业务一律使用 EDBAZodiac / EDBAElement / EDBAFiveCamp；不得再引入 Faction 或 DivinePantheon。
 */
namespace DBAIdentityTypeAdapter
{
	DIVINEBEASTSARENA_API EDBAZodiac ToCanonical(EDBAZodiacType ZodiacType);
	DIVINEBEASTSARENA_API EDBAZodiacType ToLegacy(EDBAZodiac Zodiac);
	DIVINEBEASTSARENA_API EDBAElement ToCanonical(EDBAElementType ElementType);
	DIVINEBEASTSARENA_API EDBAElementType ToLegacy(EDBAElement Element);
	DIVINEBEASTSARENA_API EDBAFiveCamp ToCanonical(EDBAFiveCampType FiveCampType);
	DIVINEBEASTSARENA_API EDBAFiveCampType ToLegacy(EDBAFiveCamp FiveCamp);
}
