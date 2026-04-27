// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DBAStaticDataAsset.generated.h"

class UDataTable;

/**
 * 静态游戏数据资产
 * 集中管理所有静态数据表的引用，提供统一访问入口
 * 用于 GameInstance / Subsystem 初始化时加载静态数据
 * Dedicated Server 必须加载此资产以获取游戏规则数据
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAStaticDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UDBAStaticDataAsset();

	/** 生肖静态数据表引用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StaticData|Zodiac", meta = (DisplayName = "生肖数据表"))
	TSoftObjectPtr<UDataTable> ZodiacStaticTable;

	/** 自然元素之力定义数据表引用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StaticData|Element", meta = (DisplayName = "自然元素之力数据表"))
	TSoftObjectPtr<UDataTable> ElementDefinitionTable;

	/** 五大阵营显示数据表引用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StaticData|FiveCamp", meta = (DisplayName = "五大阵营数据表"))
	TSoftObjectPtr<UDataTable> FiveCampDisplayTable;

	/** 地图定义数据表引用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StaticData|Map", meta = (DisplayName = "地图数据表"))
	TSoftObjectPtr<UDataTable> MapDefinitionTable;

	/** 游戏模式定义数据表引用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StaticData|Mode", meta = (DisplayName = "模式数据表"))
	TSoftObjectPtr<UDataTable> ModeDefinitionTable;

	/**
	 * 预加载所有静态数据表
	 * 在 GameInstance 初始化时调用
	 * Dedicated Server 必须调用此方法以加载游戏规则
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticData")
	void PreloadAllTables();

	/**
	 * 验证所有静态数据表是否有效
	 * 用于编辑器工具、自动化测试、启动检查
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticData")
	bool ValidateAllTables() const;

	/**
	 * 获取生肖静态数据表
	 * 同步加载，仅在已预加载或编辑器模式下使用
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticData|Zodiac")
	UDataTable* GetZodiacStaticTable() const;

	/**
	 * 获取自然元素之力定义数据表
	 * 同步加载，仅在已预加载或编辑器模式下使用
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticData|Element")
	UDataTable* GetElementDefinitionTable() const;

	/**
	 * 获取五大阵营显示数据表
	 * 同步加载，仅在已预加载或编辑器模式下使用
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticData|FiveCamp")
	UDataTable* GetFiveCampDisplayTable() const;

	/**
	 * 获取地图定义数据表
	 * 同步加载，仅在已预加载或编辑器模式下使用
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticData|Map")
	UDataTable* GetMapDefinitionTable() const;

	/**
	 * 获取游戏模式定义数据表
	 * 同步加载，仅在已预加载或编辑器模式下使用
	 */
	UFUNCTION(BlueprintCallable, Category = "StaticData|Mode")
	UDataTable* GetModeDefinitionTable() const;

#if WITH_EDITOR
	/**
	 * 编辑器下验证数据完整性
	 * 检查所有表引用是否有效、行数据是否符合规范
	 */
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
};
