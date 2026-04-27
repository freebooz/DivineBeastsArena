// Copyright FreeboozStudio. All Rights Reserved.

#include "GAS/DBAAbilitySetLibrary.h"
#include "Engine/AssetManager.h"
#include "Common/DBALogChannels.h"

UDBAFixedSkillGroupDataAsset* UDBAFixedSkillGroupLibrary::GetFixedSkillGroupById(const FName& FixedSkillGroupId)
{
	if (FixedSkillGroupId.IsNone())
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] GetFixedSkillGroupById 失败：FixedSkillGroupId 为空"));
		return nullptr;
	}

	// UE 5.7: 使用 AssetManager 异步加载
	// 此处返回 nullptr，实际使用时通过 DataAsset 蓝图表配置
	UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] GetFixedSkillGroupById 需要通过 AssetManager 异步加载：%s"), *FixedSkillGroupId.ToString());
	return nullptr;
}