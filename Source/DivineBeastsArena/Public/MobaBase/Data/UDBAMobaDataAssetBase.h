// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Data/DBADataAssetBase.h"
#include "UDBAMobaDataAssetBase.generated.h"

/**
 * MOBA 游戏 DataAsset 基类
 *
 * 提供 MOBA 游戏通用的数据资产功能：
 * - 阵营/队伍关联
 * - 游戏模式关联
 * - 资源路径管理
 */
UCLASS(Abstract, BlueprintType)
class DIVINEBEASTSARENA_API UDBAMobaDataAssetBase : public UDBADataAssetBase
{
	GENERATED_BODY()

public:
	UDBAMobaDataAssetBase(const FObjectInitializer& ObjectInitializer);

	/**
	 * 获取资产分类
	 */
	UFUNCTION(BlueprintCallable, Category = "MobaBase|Data")
	FName GetAssetCategory() const { return AssetCategory; }

protected:
	/** 资产分类标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MobaBase|Data")
	FName AssetCategory;
};
