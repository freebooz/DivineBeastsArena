// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DBAMapDefinition.generated.h"

/**
 * 地图定义行
 * 定义地图的关卡路径、显示名称、描述、加载图、支持模式、支持平台
 * 用于前台地图选择、加载流程、Travel 流转
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMapDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 地图唯一标识符（例如 "L_Arena_5v5_01"） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map", meta = (DisplayName = "地图标识符"))
	FName MapId;

	/** 地图中文显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map", meta = (DisplayName = "中文名称"))
	FText DisplayNameCN;

	/** 地图英文显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map", meta = (DisplayName = "英文名称"))
	FText DisplayNameEN;

	/** 地图描述 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map", meta = (DisplayName = "描述", MultiLine = true))
	FText Description;

	/** 地图关卡路径（软引用） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map", meta = (DisplayName = "关卡路径"))
	TSoftObjectPtr<UWorld> LevelAsset;

	/** 地图加载图路径（PC 高清版） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Loading", meta = (DisplayName = "加载图路径(PC)"))
	TSoftObjectPtr<UTexture2D> LoadingScreenTexture;

	/** 地图加载图路径（Android 低配版） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Loading", meta = (DisplayName = "加载图路径(Android)"))
	TSoftObjectPtr<UTexture2D> LoadingScreenTexture_Mobile;

	/** 地图缩略图路径（用于地图选择界面） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map", meta = (DisplayName = "缩略图路径"))
	TSoftObjectPtr<UTexture2D> ThumbnailTexture;

	/** 地图类型（Frontend / Arena / Practice / Test） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map", meta = (DisplayName = "地图类型"))
	FName MapType;

	/** 支持的游戏模式列表（例如 "5v5", "3v3", "Practice"） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map", meta = (DisplayName = "支持模式列表"))
	TArray<FName> SupportedModes;

	/** 是否支持 PC 平台 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Platform", meta = (DisplayName = "支持PC"))
	bool bSupportPC = true;

	/** 是否支持 Android 平台 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Platform", meta = (DisplayName = "支持Android"))
	bool bSupportAndroid = true;

	/** 是否支持 Linux Dedicated Server */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Platform", meta = (DisplayName = "支持Linux DS"))
	bool bSupportLinuxDS = true;

	/** 是否在当前版本可用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map", meta = (DisplayName = "是否可用"))
	bool bIsAvailable = true;

	/** 最小玩家数量要求 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map", meta = (DisplayName = "最小玩家数"))
	int32 MinPlayers = 1;

	/** 最大玩家数量限制 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map", meta = (DisplayName = "最大玩家数"))
	int32 MaxPlayers = 10;

	FDBAMapDefinitionRow()
		: MapId(NAME_None)
		, MapType(NAME_None)
		, bSupportPC(true)
		, bSupportAndroid(true)
		, bSupportLinuxDS(true)
		, bIsAvailable(true)
		, MinPlayers(1)
		, MaxPlayers(10)
	{
	}
};
