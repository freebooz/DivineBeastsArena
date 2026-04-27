// Copyright FreeboozStudio. All Rights Reserved.

#include "MobaBase/GAS/DBAMobaAbilityTagRelationshipMap.h"

void UDBAMobaAbilityTagRelationshipMap::GetBlockTagsForAbility(FGameplayTag AbilityTag, FGameplayTagContainer& OutBlockTags) const
{
	// 重置输出容器
	OutBlockTags.Reset();

	// 遍历所有关系配置，查找匹配当前 AbilityTag 的阻止关系
	for (const FDBAMobaAbilityTagRelationshipConfig& Config : Relationships)
	{
		if (Config.AbilityTag.MatchesTagExact(AbilityTag) && Config.RelationshipType == EDBAMobaAbilityTagRelationship::Block)
		{
			// 添加阻止 Tag 到输出容器
			OutBlockTags.AppendTags(Config.TargetTags);
		}
	}
}

void UDBAMobaAbilityTagRelationshipMap::GetCancelTagsForAbility(FGameplayTag AbilityTag, FGameplayTagContainer& OutCancelTags) const
{
	// 重置输出容器
	OutCancelTags.Reset();

	// 遍历所有关系配置，查找匹配当前 AbilityTag 的取消关系
	for (const FDBAMobaAbilityTagRelationshipConfig& Config : Relationships)
	{
		if (Config.AbilityTag.MatchesTagExact(AbilityTag) && Config.RelationshipType == EDBAMobaAbilityTagRelationship::Cancel)
		{
			// 添加取消 Tag 到输出容器
			OutCancelTags.AppendTags(Config.TargetTags);
		}
	}
}

bool UDBAMobaAbilityTagRelationshipMap::IsAbilityBlocked(FGameplayTag AbilityTag, const FGameplayTagContainer& CurrentTags) const
{
	// 获取该技能的阻止 Tag
	FGameplayTagContainer BlockTags;
	GetBlockTagsForAbility(AbilityTag, BlockTags);

	// 检查当前 Tag 是否包含任何阻止 Tag
	return CurrentTags.HasAny(BlockTags);
}
