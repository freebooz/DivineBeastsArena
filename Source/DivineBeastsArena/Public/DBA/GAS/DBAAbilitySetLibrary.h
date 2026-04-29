// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataAsset.h"
#include "DBA/GAS/Abilities/DBAZodiacAbilityBase.h"
#include "DBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "DBA/GAS/Abilities/DBAZodiacUltimateAbilityBase.h"
#include "DBA/GAS/Abilities/DBAResonanceAbilityBase.h"
#include "DBAAbilitySetLibrary.generated.h"

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
};

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
	 * 通过 FixedSkillGroupId 查询对应的 AbilitySet
	 * 若使用 AssetManager，此处会进行 Soft 引用加载或同步获取
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|FixedSkillGroup")
	static UDBAFixedSkillGroupDataAsset* GetFixedSkillGroupById(const FName& FixedSkillGroupId);
};
