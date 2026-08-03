// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameCore/Data/DBADataAssetBase.h"
#include "GameCore/Types/DBACommonTypes.h"
#include "GameDBA/Core/Interfaces/DBAValidatableInterface.h"
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
class DIVINEBEASTSARENA_API UDBAAbilitySetDataAsset : public UDBADataAssetBase, public IDBAValidatableInterface
{
	GENERATED_BODY()

public:
	/** DataTable 加载完成事件。参数为加载完成的 DataTable 指针与软引用路径。 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDataTableLoaded, UDataTable*, const FSoftObjectPath&);

	UDBAAbilitySetDataAsset();

	//~ IDBAValidatableInterface 实现
	virtual bool ValidateData_Implementation(TArray<FString>& OutErrors) const override;
	//~ IDBAValidatableInterface 实现 结束

	/** DataTable 加载完成事件广播。UI 可订阅此事件驱动更新，避免轮询。 */
	FOnDataTableLoaded OnDataTableLoaded;

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

	/**
	 * 异步预加载全部技能组数据表。
	 */
	UFUNCTION(BlueprintCallable, Category = "AbilitySetData")
	void PreloadAllDataTablesAsync() const;

protected:
	/**
	 * 读取已经加载完成的数据表；未加载时只发起异步加载请求，不阻塞线程。
	 *
	 * @param DataTablePtr 数据表软引用
	 * @return 已加载的数据表，尚未加载或失败返回 nullptr
	 */
	UDataTable* LoadDataTable(const TSoftObjectPtr<UDataTable>& DataTablePtr) const;

	void RequestDataTableAsync(const TSoftObjectPtr<UDataTable>& DataTablePtr) const;

	bool ValidateLoadedTableRowCount(
		const TSoftObjectPtr<UDataTable>& DataTablePtr,
		const TCHAR* TableLabel,
		int32 ExpectedRowCount,
		const TCHAR* ExpectedDescription,
		TArray<FString>& OutErrors) const;

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

private:
	mutable TSet<FSoftObjectPath> RequestedDataTableLoads;
};
