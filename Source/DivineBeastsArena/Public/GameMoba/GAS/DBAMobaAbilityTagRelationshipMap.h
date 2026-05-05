// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DBAMobaAbilityTagRelationshipMap.generated.h"

/**
 * Ability Tag 关系类型
 * 定义 Tag 之间的阻止 / 取消关系
 */
UENUM(BlueprintType)
enum class EDBAMobaAbilityTagRelationship : uint8
{
	/** 阻止激活（拥有 BlockTag 时无法激活 AbilityTag） */
	Block UMETA(DisplayName = "阻止激活"),

	/** 取消激活（激活 AbilityTag 时取消 CancelTag） */
	Cancel UMETA(DisplayName = "取消激活")
};

/**
 * Ability Tag 关系配置
 * 定义单个 Tag 关系规则
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMobaAbilityTagRelationshipConfig
{
	GENERATED_BODY()

	/** Ability Tag */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tag Relationship", meta = (DisplayName = "Ability Tag"))
	FGameplayTag AbilityTag;

	/** 关系类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tag Relationship", meta = (DisplayName = "关系类型"))
	EDBAMobaAbilityTagRelationship RelationshipType = EDBAMobaAbilityTagRelationship::Block;

	/** 目标 Tag 容器（Block / Cancel 的目标） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tag Relationship", meta = (DisplayName = "目标 Tag"))
	FGameplayTagContainer TargetTags;
};

/**
 * Ability Tag 关系映射表
 * 集中管理所有 Ability Tag 之间的阻止 / 取消关系
 *
 * 使用场景：
 * - 定义技能互斥关系（如：冲锋时无法释放其他技能）
 * - 定义技能打断关系（如：眩晕打断施法）
 * - 定义技能优先级关系（如：大招可以打断普通技能）
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAMobaAbilityTagRelationshipMap : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Tag 关系配置列表 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tag Relationship", meta = (DisplayName = "Tag 关系列表"))
	TArray<FDBAMobaAbilityTagRelationshipConfig> Relationships;

	/**
	 * 查询 Ability Tag 的阻止 Tag
	 * @param AbilityTag Ability Tag
	 * @param OutBlockTags 输出的阻止 Tag 容器
	 */
	UFUNCTION(BlueprintCallable, Category = "Tag Relationship")
	void GetBlockTagsForAbility(FGameplayTag AbilityTag, FGameplayTagContainer& OutBlockTags) const;

	/**
	 * 查询 Ability Tag 的取消 Tag
	 * @param AbilityTag Ability Tag
	 * @param OutCancelTags 输出的取消 Tag 容器
	 */
	UFUNCTION(BlueprintCallable, Category = "Tag Relationship")
	void GetCancelTagsForAbility(FGameplayTag AbilityTag, FGameplayTagContainer& OutCancelTags) const;

	/**
	 * 判断 Ability Tag 是否被阻止
	 * @param AbilityTag Ability Tag
	 * @param CurrentTags 当前拥有的 Tag 容器
	 * @return 是否被阻止
	 */
	UFUNCTION(BlueprintCallable, Category = "Tag Relationship")
	bool IsAbilityBlocked(FGameplayTag AbilityTag, const FGameplayTagContainer& CurrentTags) const;
};
