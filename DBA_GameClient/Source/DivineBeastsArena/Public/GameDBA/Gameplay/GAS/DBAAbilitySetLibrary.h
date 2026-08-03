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
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameCore/Data/DBADataAssetBase.h"
#include "GameDBA/Gameplay/Abilities/DBAZodiacAbilityBase.h"
#include "GameDBA/Gameplay/Abilities/DBAElementAbilityBase.h"
#include "GameDBA/Gameplay/Abilities/DBAZodiacUltimateAbilityBase.h"
#include "GameDBA/Gameplay/Abilities/DBAResonanceAbilityBase.h"
#include "InputCoreTypes.h"
#include "DBAAbilitySetLibrary.generated.h"

class UDataTable;
class UGameplayEffect;
class UTexture2D;

/**
 * 固定技能组中单个可施放技能的运行配置。
 *
 * 说明：
 * - C++ 读取、校验并应用该配置。
 * - DataAsset / Blueprint 只填写参数与资源引用。
 * - 不在 Ability CDO 中写死消耗、冷却或 UI 文案。
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAAbilityRuntimeConfig
{
	GENERATED_BODY()

	/** 技能显示名称，用于 HUD、事件流和调试展示 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|AbilityRuntime")
	FText DisplayName;

	/** 技能图标，用于 HUD 技能槽 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|AbilityRuntime")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 普通能量消耗；具体扣减由 C++ GAS 流程执行 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|AbilityRuntime", meta = (ClampMin = "0.0"))
	float EnergyCost = 0.0f;

	/** 可选消耗 GameplayEffect；未配置时走项目默认能量扣减路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|AbilityRuntime")
	TSubclassOf<UGameplayEffect> CostGameplayEffectClass;

	/** 冷却时长；由 C++ 创建或应用 Cooldown GE */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|AbilityRuntime|Cooldown", meta = (ClampMin = "0.0"))
	float CooldownDuration = 0.0f;

	/** 可选专用冷却 GameplayEffect；未配置时走项目默认 UDBAGE_Cooldown 路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|AbilityRuntime|Cooldown")
	TSubclassOf<UGameplayEffect> CooldownGameplayEffectClass;

	/** 冷却标签；用于 GAS 查询和 HUD 事件镜像 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|AbilityRuntime|Cooldown", meta = (Categories = "Cooldown"))
	FGameplayTag CooldownTag;

	bool Validate(FStringView SlotName, TArray<FString>& OutErrors) const;
};

/**
 * 技能组资源配置
 * 承载 FixedSkillGroup 数据
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAFixedSkillGroupDataAsset : public UDBADataAssetBase
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|FixedSkillGroup|Runtime")
	FDBAAbilityRuntimeConfig Skill01RuntimeConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|FixedSkillGroup|Runtime")
	FDBAAbilityRuntimeConfig Skill02RuntimeConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|FixedSkillGroup|Runtime")
	FDBAAbilityRuntimeConfig Skill03RuntimeConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|FixedSkillGroup|Runtime")
	FDBAAbilityRuntimeConfig Skill04RuntimeConfig;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey PassiveInputKey = EKeys::Invalid;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey Skill01InputKey = EKeys::Q;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey Skill02InputKey = EKeys::W;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey Skill03InputKey = EKeys::E;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey Skill04InputKey = EKeys::Invalid;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey ZodiacUltimateInputKey = EKeys::R;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|FixedSkillGroup|Input")
	FKey ResonanceInputKey = EKeys::Invalid;

	bool ValidateRuntimeAbilityConfigs(TArray<FString>& OutErrors) const;
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
	 * 通过 FixedSkillGroupId 查询已预加载/已缓存的 AbilitySet；未加载时排队异步预加载并返回运行时兜底
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
