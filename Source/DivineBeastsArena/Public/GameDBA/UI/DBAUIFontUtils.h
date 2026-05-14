// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"

class UWidgetTree;

namespace DBAUIFonts
{
	DIVINEBEASTSARENA_API FSlateFontInfo MakeGameFont(float Size, int32 OutlineSize = 1);
	DIVINEBEASTSARENA_API void ApplyGameFontToWidgetTree(UWidgetTree* WidgetTree);
}
