// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Common/Types/DBACommonTypes.h"
#include "DBAAbilitySetDataAsset.generated.h"

class UDataTable;

/**
 * 技能组汇总数据行引用
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAAbilitySetSummaryRef
{
	GENERATED_BODY()

	/** 技能组汇总数据表行名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySet")
	FName SummaryRowName;

	/** 生肖类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySet")
	EDBAZodiac Zodiac = EDBAZodiac::None;
};

/**
 * 技能组数据资产
 *
 * 用途：
 * - 集中管理所有技能组数据表
 * - 提供统一的数据查询接口
 * - 支持技能组汇总查询
 *
 * 数据表：
 * - ElementPassiveTable：元素被动技能数据表（5 行）
 * - ElementActiveAbilityTable：元素主动技能数据表（25 行）
 * - ElementUltimateTemplateTable：元素终极技能模板数据表（5 行）
 * - ElementResonanceTable：元素共鸣数据表（25 行）
 * - ZodiacUltimateTable：生肖终极技能数据表（12 行）
 * - SkillDataTable：统一技能数据表
 * - AbilitySetSummaryTable：技能组汇总数据表（12 行）
 *
 * 使用方式：
 * 1. 创建 DataAsset 实例
 * 2. 关联 DataTable 资产
 * 3. 在 GameInstance / Subsystem 中加载
 * 4. 通过接口查询数据
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAAbilitySetDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UDBAAbilitySetDataAsset();

	/**
	 * 获取元素被动技能数据
	 *
	 * @param Element 元素类型
	 * @param OutRow 输出数据行
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "AbilitySetData")
	bool GetElementPassiveData(EDBAElement Element, FName& OutRowName) const;

	/**
	 * 获取元素主动技能数据
	 *
	 * @param Element 元素类型
	 * @param SkillPosition 技能位置（0-4）
	 * @param OutRowName 输出数据行名称
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "AbilitySetData")
	bool GetElementActiveAbilityData(EDBAElement Element, int32 SkillPosition, FName& OutRowName) const;

	/**
	 * 获取元素终极技能模板数据
	 *
	 * @param Element 元素类型
	 * @param OutRowName 输出数据行名称
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "AbilitySetData")
	bool GetElementUltimateTemplateData(EDBAElement Element, FName& OutRowName) const;

	/**
	 * 获取元素共鸣数据
	 *
	 * @param Element 元素类型
	 * @param ResonanceLevel 共鸣等级（0-4）
	 * @param OutRowName 输出数据行名称
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "AbilitySetData")
	bool GetElementResonanceData(EDBAElement Element, int32 ResonanceLevel, FName& OutRowName) const;

	/**
	 * 获取生肖终极技能数据
	 *
	 * @param Zodiac 生肖类型
	 * @param OutRowName 输出数据行名称
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "AbilitySetData")
	bool GetZodiacUltimateData(EDBAZodiac Zodiac, FName& OutRowName) const;

	/**
	 * 获取技能组汇总数据
	 *
	 * @param Zodiac 生肖类型
	 * @param OutRowName 输出数据行名称
	 * @return 是否找到数据
	 */
	UFUNCTION(BlueprintCallable, Category = "AbilitySetData")
	bool GetAbilitySetSummaryData(EDBAZodiac Zodiac, FName& OutRowName) const;

	/**
	 * 验证数据完整性
	 *
	 * @param OutErrors 输出错误信息列表
	 * @return 是否验证通过
	 */
	UFUNCTION(BlueprintCallable, Category = "AbilitySetData")
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
	 * 构建元素技能行名称
	 *
	 * @param Element 元素类型
	 * @return 行名称
	 */
	FName BuildElementRowName(EDBAElement Element) const;

	/**
	 * 构建元素技能位置行名称
	 *
	 * @param Element 元素类型
	 * @param Position 技能位置
	 * @return 行名称
	 */
	FName BuildElementSkillRowName(EDBAElement Element, int32 Position) const;

	/**
	 * 构建共鸣行名称
	 *
	 * @param Element 元素类型
	 * @param ResonanceLevel 共鸣等级
	 * @return 行名称
	 */
	FName BuildResonanceRowName(EDBAElement Element, int32 ResonanceLevel) const;

	/**
	 * 构建生肖行名称
	 *
	 * @param Zodiac 生肖类型
	 * @return 行名称
	 */
	FName BuildZodiacRowName(EDBAZodiac Zodiac) const;

public:
	/** 元素被动技能数据表 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> ElementPassiveTable;

	/** 元素主动技能数据表（5元素×5位置） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> ElementActiveAbilityTable;

	/** 元素终极技能模板数据表 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> ElementUltimateTemplateTable;

	/** 元素共鸣数据表 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> ElementResonanceTable;

	/** 生肖终极技能数据表 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> ZodiacUltimateTable;

	/** 统一技能数据表 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> SkillDataTable;

	/** 技能组汇总数据表 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataTables")
	TSoftObjectPtr<UDataTable> AbilitySetSummaryTable;
};
