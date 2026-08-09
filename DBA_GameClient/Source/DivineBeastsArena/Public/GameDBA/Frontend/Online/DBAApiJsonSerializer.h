// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FJsonObject;

/** Domain Service 专用 JSON 编解码辅助；Widget 不得持有或解析原始 JSON。 */
class DIVINEBEASTSARENA_API FDBAApiJsonSerializer
{
public:
	static bool SerializeObject(const TSharedPtr<FJsonObject>& Object, FString& OutJson);
	static bool DeserializeObject(const FString& Json, TSharedPtr<FJsonObject>& OutObject);
};
