// Copyright FreeboozStudio. All Rights Reserved.
// 固定技能组库实现 - 提供技能组数据的查询接口

#include "DBA/GAS/DBAAbilitySetLibrary.h"
#include "Engine/AssetManager.h"
#include "Common/DBALogChannels.h"
#include "Common/DBAConstants.h"

UDBAFixedSkillGroupDataAsset* UDBAFixedSkillGroupLibrary::GetFixedSkillGroupById(const FName& FixedSkillGroupId)
{
	// 检查传入的ID是否有效
	if (FixedSkillGroupId.IsNone())
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] GetFixedSkillGroupById 失败：FixedSkillGroupId 为空"));
		return nullptr;
	}

	// 构建资产路径
	const FSoftObjectPath AssetPath(FString::Printf(TEXT("%s/%s.%s"), DBAPaths::DT_FixedSkillGroups, *FixedSkillGroupId.ToString(), *FixedSkillGroupId.ToString()));

	// 同步加载：如果资产已在内存中则立即返回
	if (UAssetManager* AssetMgr = UAssetManager::GetIfValid())
	{
		if (UObject* Loaded = AssetMgr->GetStreamableManager().LoadSynchronous(AssetPath, false))
		{
			UE_LOG(LogDBAData, Verbose, TEXT("[UDBAFixedSkillGroupLibrary] 直接加载技能组：%s"), *FixedSkillGroupId.ToString());
			return Cast<UDBAFixedSkillGroupDataAsset>(Loaded);
		}
	}

	// 记录警告（建议使用异步加载版本）
	UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] 技能组未找到，请使用异步加载：%s"), *FixedSkillGroupId.ToString());
	return nullptr;
}

void UDBAFixedSkillGroupLibrary::LoadFixedSkillGroupByIdAsync(const FName& FixedSkillGroupId, FDBAOnFixedSkillGroupLoaded OnLoadedDelegate)
{
	// 检查传入的ID是否有效
	if (FixedSkillGroupId.IsNone())
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] LoadFixedSkillGroupByIdAsync 失败：FixedSkillGroupId 为空"));
		OnLoadedDelegate.ExecuteIfBound(nullptr);
		return;
	}

	if (!UAssetManager::Get().IsValid())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAFixedSkillGroupLibrary] AssetManager 不可用"));
		OnLoadedDelegate.ExecuteIfBound(nullptr);
		return;
	}

	// 构建资产路径
	const FSoftObjectPath AssetPath(FString::Printf(TEXT("%s/%s.%s"), DBAPaths::DT_FixedSkillGroups, *FixedSkillGroupId.ToString(), *FixedSkillGroupId.ToString()));

	UE_LOG(LogDBAData, Log, TEXT("[UDBAFixedSkillGroupLibrary] 开始异步加载技能组：%s"), *FixedSkillGroupId.ToString());

	// 使用 StreamableManager 异步加载
	TSharedPtr<FStreamableHandle, ESPMode::ThreadSafe> Handle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		TArray<FSoftObjectPath>({ AssetPath }),
		[OnLoadedDelegate, FixedSkillGroupId]()
		{
			// 异步加载完成回调
			if (UAssetManager* AssetMgr = UAssetManager::GetIfValid())
			{
				const FSoftObjectPath LoadedPath(FString::Printf(TEXT("%s/%s.%s"), DBAPaths::DT_FixedSkillGroups, *FixedSkillGroupId.ToString(), *FixedSkillGroupId.ToString()));
				if (UObject* LoadedObj = LoadedPath.ResolveObject())
				{
					if (UDBAFixedSkillGroupDataAsset* LoadedAsset = Cast<UDBAFixedSkillGroupDataAsset>(LoadedObj))
					{
						UE_LOG(LogDBAData, Log, TEXT("[UDBAFixedSkillGroupLibrary] 异步加载完成：%s"), *FixedSkillGroupId.ToString());
						OnLoadedDelegate.ExecuteIfBound(LoadedAsset);
						return;
					}
				}
			}

			UE_LOG(LogDBAData, Error, TEXT("[UDBAFixedSkillGroupLibrary] 异步加载回调失败：%s"), *FixedSkillGroupId.ToString());
			OnLoadedDelegate.ExecuteIfBound(nullptr);
		},
		FStreamableManager::AsyncLoadHighPriority
	);

	if (!Handle.IsValid())
	{
		UE_LOG(LogDBAData, Error, TEXT("[UDBAFixedSkillGroupLibrary] 异步加载请求失败：%s"), *FixedSkillGroupId.ToString());
		OnLoadedDelegate.ExecuteIfBound(nullptr);
	}
}
