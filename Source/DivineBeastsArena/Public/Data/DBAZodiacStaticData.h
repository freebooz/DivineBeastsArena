// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DBAZodiacStaticData.generated.h"

/**
 * 生肖静态数据行
 * 定义十二生肖的基础信息、显示名称、描述、图标路径
 * 用于前台选择界面、对局 HUD、结算界面显示
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAZodiacStaticRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 生肖枚举值（对应 EDBAZodiac） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "生肖枚举"))
	uint8 ZodiacEnum = 0;

	/** 生肖中文显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "中文名称"))
	FText DisplayNameCN;

	/** 生肖英文显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "英文名称"))
	FText DisplayNameEN;

	/** 生肖描述文本 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "描述", MultiLine = true))
	FText Description;

	/** 生肖图标路径（软引用） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "图标路径"))
	TSoftObjectPtr<UTexture2D> IconTexture;

	/** 生肖剪影路径（用于选择界面背景） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "剪影路径"))
	TSoftObjectPtr<UTexture2D> SilhouetteTexture;

	/** 生肖大招名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "生肖大招名称"))
	FText UltimateAbilityName;

	/** 生肖大招描述 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "生肖大招描述", MultiLine = true))
	FText UltimateAbilityDescription;

	/** 生肖大招图标路径 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "生肖大招图标"))
	TSoftObjectPtr<UTexture2D> UltimateAbilityIcon;

	/** 生肖主题色（用于 UI 高亮） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "主题色"))
	FLinearColor ThemeColor = FLinearColor::White;

	/** 是否在当前版本可用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "是否可用"))
	bool bIsAvailable = true;

	/** 解锁等级要求（0 表示默认解锁） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac", meta = (DisplayName = "解锁等级"))
	int32 UnlockLevel = 0;

	FDBAZodiacStaticRow()
		: ZodiacEnum(0)
		, ThemeColor(FLinearColor::White)
		, bIsAvailable(true)
		, UnlockLevel(0)
	{
	}
};
