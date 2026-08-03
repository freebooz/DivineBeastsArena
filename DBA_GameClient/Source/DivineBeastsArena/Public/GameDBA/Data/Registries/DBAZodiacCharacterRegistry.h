// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明 UDBAZodiacCharacterRegistry 数据资产，承载生肖到角色类的映射，替代硬编码 switch-case。
- 阅读重点：先看 UCLASS 配置和 UPROPERTY 字段，理解数据资产如何在蓝图中配置生肖角色类映射。
- 修改提示：新增生肖时只需在数据资产中添加映射条目，无需修改 C++ 代码；保持数据驱动原则。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Data/DBADataAssetBase.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAZodiacCharacterRegistry.generated.h"

class ADBAZodiacCharacterBase;
class UAnimationAsset;
class UAnimInstance;
class UMaterialInterface;
class USkeletalMesh;

/** 单个生肖角色在选角、创建和大厅阶段使用的表现资源。 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAZodiacCharacterPresentationDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Zodiac|Presentation")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Zodiac|Presentation")
	TSoftClassPtr<UAnimInstance> AnimationBlueprintClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Zodiac|Presentation")
	TSoftObjectPtr<UAnimationAsset> IdleAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Zodiac|Presentation")
	TSoftObjectPtr<UMaterialInterface> Material;
};

/**
 * UDBAZodiacCharacterRegistry
 * 生肖角色类注册表数据资产
 * 承载 EDBAZodiac 到 TSubclassOf<ADBAZodiacCharacterBase> 的映射
 * 替代 DBAGameModeBase::ResolveLobbyPawnClass 中的硬编码 switch-case
 */
UCLASS(BlueprintType, Const)
class DIVINEBEASTSARENA_API UDBAZodiacCharacterRegistry : public UDBADataAssetBase
{
	GENERATED_BODY()

public:
	/**
	 * 根据生肖类型查询角色类
	 * @param Zodiac 生肖类型
	 * @return 对应的角色类指针，未配置时返回 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "DBA|Zodiac|Registry")
	TSubclassOf<ADBAZodiacCharacterBase> GetCharacterClassForZodiac(EDBAZodiac Zodiac) const;

	/** 查询选角、创建和大厅统一使用的生肖表现定义。 */
	UFUNCTION(BlueprintPure, Category = "DBA|Zodiac|Registry")
	bool GetPresentationDefinitionForZodiac(EDBAZodiac Zodiac, FDBAZodiacCharacterPresentationDefinition& OutDefinition) const;

	/**
	 * 获取所有已注册的生肖角色类映射
	 * @return 生肖到角色类的映射表
	 */
	const TMap<EDBAZodiac, TSubclassOf<ADBAZodiacCharacterBase>>& GetZodiacCharacterClassMap() const
	{
		return ZodiacCharacterClassMap;
	}

protected:
	/**
	 * 生肖到角色类的映射表
	 * 在蓝图数据资产中配置每个生肖对应的角色类
	 * 替代原 DBAGameModeBase::ResolveLobbyPawnClass 中的 switch-case 硬编码
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Zodiac|Registry")
	TMap<EDBAZodiac, TSubclassOf<ADBAZodiacCharacterBase>> ZodiacCharacterClassMap;

	/** 生肖到表现资源的映射。所有玩家展示资源必须由此数据资产提供。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Zodiac|Registry")
	TMap<EDBAZodiac, FDBAZodiacCharacterPresentationDefinition> ZodiacPresentationMap;
};
