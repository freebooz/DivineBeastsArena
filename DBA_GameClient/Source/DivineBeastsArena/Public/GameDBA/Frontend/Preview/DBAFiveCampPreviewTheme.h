// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAFiveCampPreviewTheme.generated.h"

class UMaterialInterface;
class USoundBase;
class UTexture2D;

/**
 * 已完成软资源解析的五营前台主题投影。
 *
 * 此结构只在 Frontend PreviewStage 使用：BackgroundTexture 与 EmblemTexture 供场景蓝图放置到背景/徽记锚点，
 * VfxMaterial 供其已有 Niagara 或材质实例应用主题色，ThemeSound 仅作选择表现音效。它绝不承载 TeamId、
 * 角色战斗属性、GAS 技能或网络复制数据，因此不会影响正式 GameplayCharacter 的外观组件。
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAFiveCampPreviewTheme
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Preview|FiveCamp")
	EDBAFiveCamp FiveCamp = EDBAFiveCamp::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Preview|FiveCamp")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Preview|FiveCamp")
	FLinearColor ThemeColor = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Preview|FiveCamp")
	FLinearColor SecondaryColor = FLinearColor::Gray;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Preview|FiveCamp")
	TObjectPtr<UTexture2D> BackgroundTexture = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Preview|FiveCamp")
	TObjectPtr<UTexture2D> EmblemTexture = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Preview|FiveCamp")
	TObjectPtr<UMaterialInterface> VfxMaterial = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Preview|FiveCamp")
	TObjectPtr<USoundBase> ThemeSound = nullptr;
};
