// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "DBAFiveCampDisplayData.generated.h"

/**
 * 五大阵营显示数据行
 * 定义五大阵营的外观、特效色调、法相表现、UI 图标风格、声音风格
 * 五大阵营只影响表现层，不影响自然元素之力克制、核心属性、伤害公式、技能组生成、TeamId
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAFiveCampDisplayRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 五大阵营枚举值（对应 EDBAFiveCamp） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp", meta = (DisplayName = "五大阵营枚举"))
	uint8 FiveCampEnum = 0;

	/** 五大阵营中文显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp", meta = (DisplayName = "中文名称"))
	FText DisplayNameCN;

	/** 五大阵营英文显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp", meta = (DisplayName = "英文名称"))
	FText DisplayNameEN;

	/** 五大阵营描述 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp", meta = (DisplayName = "描述", MultiLine = true))
	FText Description;

	/** 五大阵营图标路径 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp", meta = (DisplayName = "图标路径"))
	TSoftObjectPtr<UTexture2D> IconTexture;

	/** 五大阵营徽记路径（用于选择界面、结算界面） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp", meta = (DisplayName = "徽记路径"))
	TSoftObjectPtr<UTexture2D> EmblemTexture;

	/** 五大阵营法相背景路径（用于选择界面背景） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp", meta = (DisplayName = "法相背景路径"))
	TSoftObjectPtr<UTexture2D> BackgroundTexture;

	/** 五大阵营主题色（用于特效色调、UI 高亮） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp", meta = (DisplayName = "主题色"))
	FLinearColor ThemeColor = FLinearColor::White;

	/** 五大阵营次要色（用于渐变、边框） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp", meta = (DisplayName = "次要色"))
	FLinearColor SecondaryColor = FLinearColor::Gray;

	/** 五大阵营特效材质路径（用于技能特效、法相特效） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp|Visual", meta = (DisplayName = "特效材质路径"))
	TSoftObjectPtr<UMaterialInterface> EffectMaterial;

	/** 五大阵营音效主题路径（用于技能音效、UI 音效） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp|Audio", meta = (DisplayName = "音效主题路径"))
	TSoftObjectPtr<USoundCue> ThemeSound;

	/** 五大阵营选择界面背景音乐路径 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp|Audio", meta = (DisplayName = "选择界面背景音乐"))
	TSoftObjectPtr<USoundCue> SelectionMusic;

	/** 五大阵营皮肤主题标签（用于皮肤系统过滤） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp|Skin", meta = (DisplayName = "皮肤主题标签"))
	FName SkinThemeTag;

	/** 是否在当前版本可用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp", meta = (DisplayName = "是否可用"))
	bool bIsAvailable = true;

	/** 解锁等级要求（0 表示默认解锁） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FiveCamp", meta = (DisplayName = "解锁等级"))
	int32 UnlockLevel = 0;

	FDBAFiveCampDisplayRow()
		: FiveCampEnum(0)
		, ThemeColor(FLinearColor::White)
		, SecondaryColor(FLinearColor::Gray)
		, SkinThemeTag(NAME_None)
		, bIsAvailable(true)
		, UnlockLevel(0)
	{
	}
};
