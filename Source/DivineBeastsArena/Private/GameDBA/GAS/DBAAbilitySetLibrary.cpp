// Copyright FreeboozStudio. All Rights Reserved.

#include "GameDBA/GAS/DBAAbilitySetLibrary.h"

#include "Engine/AssetManager.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"

namespace
{
	FSoftObjectPath MakeFixedSkillGroupPath(const FName& FixedSkillGroupId)
	{
		const FString AssetName = FixedSkillGroupId.ToString();
		return FSoftObjectPath(FString::Printf(TEXT("%s/%s.%s"), DBAPaths::DT_FixedSkillGroups, *AssetName, *AssetName));
	}
}

UDBAFixedSkillGroupDataAsset* UDBAFixedSkillGroupLibrary::GetFixedSkillGroupById(const FName& FixedSkillGroupId)
{
	if (FixedSkillGroupId.IsNone())
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] 固定技能组 ID 为空。"));
		return nullptr;
	}

	const FSoftObjectPath AssetPath = MakeFixedSkillGroupPath(FixedSkillGroupId);
	if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		if (UObject* Loaded = AssetManager->GetStreamableManager().LoadSynchronous(AssetPath, false))
		{
			return Cast<UDBAFixedSkillGroupDataAsset>(Loaded);
		}
	}

	UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] 未找到固定技能组：%s"), *FixedSkillGroupId.ToString());
	return nullptr;
}

void UDBAFixedSkillGroupLibrary::LoadFixedSkillGroupByIdAsync(const FName& FixedSkillGroupId, FDBAOnFixedSkillGroupLoaded OnLoadedDelegate)
{
	if (FixedSkillGroupId.IsNone())
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] 固定技能组 ID 为空。"));
		OnLoadedDelegate.ExecuteIfBound(nullptr);
		return;
	}

	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager)
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAFixedSkillGroupLibrary] AssetManager 不可用。"));
		OnLoadedDelegate.ExecuteIfBound(nullptr);
		return;
	}

	const FSoftObjectPath AssetPath = MakeFixedSkillGroupPath(FixedSkillGroupId);
	AssetManager->GetStreamableManager().RequestAsyncLoad(
		TArray<FSoftObjectPath>({ AssetPath }),
		[AssetPath, FixedSkillGroupId, OnLoadedDelegate]()
		{
			UObject* LoadedObject = AssetPath.ResolveObject();
			UDBAFixedSkillGroupDataAsset* LoadedAsset = Cast<UDBAFixedSkillGroupDataAsset>(LoadedObject);
			if (!LoadedAsset)
			{
				UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] 异步加载失败：%s"), *FixedSkillGroupId.ToString());
			}
			OnLoadedDelegate.ExecuteIfBound(LoadedAsset);
		},
		FStreamableManager::AsyncLoadHighPriority);
}
