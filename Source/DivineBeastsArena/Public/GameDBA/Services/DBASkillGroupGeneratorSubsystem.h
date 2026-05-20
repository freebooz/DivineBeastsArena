// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameDBA/Data/DBAFixedSkillGroupData.h"
#include "Engine/DataTable.h"
#include "DBASkillGroupGeneratorSubsystem.generated.h"

/**
 * DBASkillGroupGeneratorSubsystem
 *
 * 固定技能组生成子系统
 *
 * 功能：
 * - 根据 Zodiac + Element 查表生成固定技能组
 * - 提供技能组查询接口
 * - 管理技能组数据表缓存
 *
 * 设计原则：
 * - 玩家不自由选择技能
 * - 技能组由 Zodiac + Element 查表自动生成
 * - FiveCamp 只改变表现包，不改变技能组
 *
 * 使用示例：
 * - 获取技能组: GetSkillGroup(Zodiac, Element)
 * - 获取技能组摘要: GetSkillGroupSummary(Zodiac)
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBASkillGroupGeneratorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDBASkillGroupGeneratorSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem interface

	/**
	 * 根据生肖和元素获取固定技能组
	 *
	 * @param Zodiac 生肖类型
	 * @param Element 元素类型
	 * @param OutSkillGroup 输出技能组数据
	 * @return 是否成功获取
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillGroup")
	bool GetSkillGroup(EDBAZodiac Zodiac, EDBAElement Element, FDBAZodiacElementFixedSkillGroupRow& OutSkillGroup) const;

	/**
	 * 根据生肖获取所有元素技能组摘要
	 *
	 * @param Zodiac 生肖类型
	 * @param OutSummary 输出技能组摘要数据
	 * @return 是否成功获取
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillGroup")
	bool GetSkillGroupSummary(EDBAZodiac Zodiac, FDBAZodiacHeroAbilitySetSummaryRow& OutSummary) const;

	/**
	 * 获取所有可用的生肖类型
	 *
	 * @return 生肖类型数组
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillGroup")
	TArray<EDBAZodiac> GetAllZodiacTypes() const;

	/**
	 * 获取所有可用的元素类型
	 *
	 * @return 元素类型数组
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillGroup")
	TArray<EDBAElement> GetAllElementTypes() const;

	/**
	 * 检查技能组是否已配置
	 *
	 * @param Zodiac 生肖类型
	 * @param Element 元素类型
	 * @return 是否已配置
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillGroup")
	bool IsSkillGroupConfigured(EDBAZodiac Zodiac, EDBAElement Element) const;

	/**
	 * 获取生肖大招技能ID
	 *
	 * @param Zodiac 生肖类型
	 * @return 大招技能ID
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillGroup")
	FName GetZodiacUltimateSkillId(EDBAZodiac Zodiac) const;

	/**
	 * 计算给定技能的共鸣等级
	 *
	 * @param SkillIds 技能ID数组
	 * @param Element 元素类型
	 * @return 共鸣等级 (0-4)
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillGroup")
	int32 CalculateResonanceLevel(const TArray<FName>& SkillIds, EDBAElement Element) const;

protected:
	/**
	 * 加载技能组数据表
	 */
	void LoadSkillGroupDataTable();

	/**
	 * 加载技能组摘要数据表
	 */
	void LoadSkillGroupSummaryDataTable();

	/**
	 * 根据数据表行名查找技能组
	 *
	 * @param RowName 行名
	 * @param OutSkillGroup 输出技能组数据
	 * @return 是否成功找到
	 */
	bool FindSkillGroupByRowName(const FName& RowName, FDBAZodiacElementFixedSkillGroupRow& OutSkillGroup) const;

private:
	/** 固定技能组数据表 */
	UPROPERTY()
	TSoftObjectPtr<UDataTable> SkillGroupDataTable;

	/** 技能组摘要数据表 */
	UPROPERTY()
	TSoftObjectPtr<UDataTable> SkillGroupSummaryDataTable;

	/** 已加载的数据表指针 */
	UPROPERTY()
	UDataTable* LoadedSkillGroupDataTable;

	/** 已加载的摘要数据表指针 */
	UPROPERTY()
	UDataTable* LoadedSkillGroupSummaryDataTable;

	/** 数据表路径 */
	static constexpr const TCHAR* SkillGroupDataTablePath = TEXT("/Game/DBA/Data/Tables/DT_FixedSkillGroups");
	static constexpr const TCHAR* SkillGroupSummaryDataTablePath = TEXT("/Game/DBA/Data/Tables/DT_ZodiacSkillGroupSummaries");
};