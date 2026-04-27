// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Common/Types/DBACommonTypes.h"
#include "Data/DBAZodiacHeroData.h"
#include "Data/DBAFixedSkillGroupData.h"
#include "DBAZodiacHeroDataAsset.generated.h"

/**
 * 生肖英雄数据资产
 *
 * 用途：
 * - 集中管理所有生肖英雄数据表
 * - 提供统一的数据查询接口
 * - 支持运行时数据加载和缓存
 *
 * 数据表：
 * - ZodiacHeroDisplayTable：生肖英雄显示数据表
 * - ZodiacHeroConfigTable：生肖英雄配置数据表
 * - FixedSkillGroupTable：固定技能组数据表
 * - AbilitySetSummaryTable：技能组汇总数据表
 *
 * 使用方式：
 * 1. 创建 DataAsset 实例
 * 2. 关联 DataTable 资产
 * 3. 在 GameInstance / Subsystem 中加载
 * 4. 通过接口查询数据
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAZodiacHeroDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UDBAZodiacHeroDataAsset();

	/**
	 * 生肖英雄显示数据表
	 * 包含 12 个生肖英雄的显示信息
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> ZodiacHeroDisplayTable;

	/**
	 * 生肖英雄配置数据表
	 * 包含 12 个生肖英雄的配置信息
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> ZodiacHeroConfigTable;

	/**
	 * 固定技能组数据表
	 * 包含 60 条固定技能组（12 生肖 × 5 元素）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> FixedSkillGroupTable;

	/**
	 * 技能组汇总数据表
	 * 包含 12 个生肖英雄的技能组汇总
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> AbilitySetSummaryTable;

	/**
	 * 获取生肖英雄显示数据
	 *
	 * @param Zodiac 生肖类型
	 * @param OutRow 输出数据行
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool GetZodiacHeroDisplayData(EDBAZodiac Zodiac, FDBAZodiacHeroDisplayRow& OutRow) const;

	/**
	 * 获取生肖英雄配置数据
	 *
	 * @param Zodiac 生肖类型
	 * @param OutRow 输出数据行
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool GetZodiacHeroConfigData(EDBAZodiac Zodiac, FDBAZodiacHeroConfigRow& OutRow) const;

	/**
	 * 获取固定技能组数据
	 *
	 * @param Zodiac 生肖类型
	 * @param Element 自然元素之力类型
	 * @param OutRow 输出数据行
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool GetFixedSkillGroupData(EDBAZodiac Zodiac, EDBAElement Element, FDBAZodiacElementFixedSkillGroupRow& OutRow) const;

	/**
	 * 获取技能组汇总数据
	 *
	 * @param Zodiac 生肖类型
	 * @param OutRow 输出数据行
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool GetAbilitySetSummaryData(EDBAZodiac Zodiac, FDBAZodiacHeroAbilitySetSummaryRow& OutRow) const;

	/**
	 * 获取所有可用的生肖英雄
	 *
	 * @param OutZodiacs 输出生肖列表
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	void GetAllAvailableZodiacs(TArray<EDBAZodiac>& OutZodiacs) const;

	/**
	 * 检查生肖英雄是否可用
	 *
	 * @param Zodiac 生肖类型
	 * @return 是否可用
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool IsZodiacAvailable(EDBAZodiac Zodiac) const;

	/**
	 * 检查技能组是否可用
	 *
	 * @param Zodiac 生肖类型
	 * @param Element 自然元素之力类型
	 * @return 是否可用
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool IsSkillGroupAvailable(EDBAZodiac Zodiac, EDBAElement Element) const;

	/**
	 * 验证数据完整性
	 * 检查所有数据表是否正确配置
	 *
	 * @param OutErrors 输出错误信息列表
	 * @return 是否验证通过
	 */
	UFUNCTION(BlueprintCallable, Category = "ZodiacHeroData")
	bool ValidateDataIntegrity(TArray<FString>& OutErrors) const;

protected:
	/**
	 * 加载数据表
	 *
	 * @param DataTablePtr 数据表软引用
	 * @return 加载的数据表，失败返回 nullptr
	 */
	UDataTable* LoadDataTable(const TSoftObjectPtr<UDataTable>& DataTablePtr) const;

	/**
	 * 查找数据行
	 *
	 * @param DataTable 数据表
	 * @param RowName 行名称
	 * @param OutRow 输出数据行
	 * @return 是否找到数据
	 */
	template<typename T>
	bool FindDataRow(UDataTable* DataTable, FName RowName, T& OutRow) const
	{
		if (!DataTable)
		{
			return false;
		}

		T* Row = DataTable->FindRow<T>(RowName, TEXT(""));
		if (!Row)
		{
			return false;
		}

		OutRow = *Row;
		return true;
	}

	/**
	 * 构建生肖行名称
	 * 格式：Zodiac_[ZodiacName]
	 */
	FName BuildZodiacRowName(EDBAZodiac Zodiac) const;

	/**
	 * 构建技能组行名称
	 * 格式：Zodiac_[ZodiacName]_Element_[ElementName]
	 */
	FName BuildSkillGroupRowName(EDBAZodiac Zodiac, EDBAElement Element) const;
};
