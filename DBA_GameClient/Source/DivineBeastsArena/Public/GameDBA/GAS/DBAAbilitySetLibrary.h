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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataAsset.h"
#include "GameDBA/GAS/Abilities/DBAZodiacAbilityBase.h"
#include "GameDBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "GameDBA/GAS/Abilities/DBAZodiacUltimateAbilityBase.h"
#include "GameDBA/GAS/Abilities/DBAResonanceAbilityBase.h"
#include "InputCoreTypes.h"
#include "DBAAbilitySetLibrary.generated.h"

class UDataTable;

/**
 * 技能组资源配置
 * 承载 FixedSkillGroup 数据
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAFixedSkillGroupDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup")
	FName FixedSkillGroupId;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup")
	TSubclassOf<UDBAZodiacAbilityBase> PassiveAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup")
	TSubclassOf<UDBAElementAbilityBase> Skill01Class;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup")
	TSubclassOf<UDBAElementAbilityBase> Skill02Class;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup")
	TSubclassOf<UDBAElementAbilityBase> Skill03Class;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup")
	TSubclassOf<UDBAElementAbilityBase> Skill04Class;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup")
	TSubclassOf<UDBAZodiacUltimateAbilityBase> ZodiacUltimateClass;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup")
	TSubclassOf<UDBAResonanceAbilityBase> ResonanceAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey PassiveInputKey = EKeys::Invalid;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey Skill01InputKey = EKeys::One;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey Skill02InputKey = EKeys::Two;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey Skill03InputKey = EKeys::Three;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey Skill04InputKey = EKeys::Four;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey ZodiacUltimateInputKey = EKeys::Five;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey ResonanceInputKey = EKeys::Invalid;
};

/**
 * 技能组异步加载完成委托
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FDBAOnFixedSkillGroupLoaded, UDBAFixedSkillGroupDataAsset*, LoadedAsset);

/**
 * 技能组静态函数库
 * 提供通过 FixedSkillGroupId 查询 AbilitySet 的能力
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAFixedSkillGroupLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 通过 FixedSkillGroupId 查询对应的 AbilitySet（同步版本）
	 * 优先从已缓存的资产中加载
	 * @param FixedSkillGroupId 技能组 ID
	 * @return 技能组数据资产，如果未找到返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|FixedSkillGroup")
	static UDBAFixedSkillGroupDataAsset* GetFixedSkillGroupById(const FName& FixedSkillGroupId);

	/**
	 * 通过 FixedSkillGroupId 异步加载对应的 AbilitySet
	 * 适用于需要动态加载技能组的场景
	 * @param FixedSkillGroupId 技能组 ID
	 * @param OnLoadedDelegate 加载完成回调
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|FixedSkillGroup")
	static void LoadFixedSkillGroupByIdAsync(const FName& FixedSkillGroupId, FDBAOnFixedSkillGroupLoaded OnLoadedDelegate);
};
