// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/GAS/DBAAbilitySetLibrary.h"

#include "Engine/AssetManager.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GAS/Abilities/DBAElementSkillAbility_Generic.h"
#include "GameDBA/GAS/Abilities/DBAZodiacPassiveAbility_Generic.h"
#include "GameDBA/GAS/Abilities/DBAZodiacUltimateAbility_Generic.h"
#include "Misc/PackageName.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	FSoftObjectPath MakeObjectPath(const FString& PackagePath, const FString& AssetName)
	{
		return FSoftObjectPath(FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName));
	}

	bool DoesObjectPackageExist(const FSoftObjectPath& AssetPath)
	{
		const FString PackageName = AssetPath.GetLongPackageName();
		return !PackageName.IsEmpty() && FPackageName::DoesPackageExist(PackageName);
	}

	int32 GetStandardSkillGroupIndex(const FString& FixedSkillGroupId)
	{
		const FString LowerId = FixedSkillGroupId.ToLower();
		if (LowerId.Contains(TEXT("_fire")))
		{
			return 1;
		}
		if (LowerId.Contains(TEXT("_water")))
		{
			return 2;
		}
		if (LowerId.Contains(TEXT("_wood")))
		{
			return 3;
		}
		if (LowerId.Contains(TEXT("_gold")) || LowerId.Contains(TEXT("_metal")))
		{
			return 4;
		}
		if (LowerId.Contains(TEXT("_earth")))
		{
			return 5;
		}
		return 1;
	}

	TArray<FSoftObjectPath> MakeFixedSkillGroupCandidatePaths(const FName& FixedSkillGroupId)
	{
		const FString AssetName = FixedSkillGroupId.ToString();
		TArray<FSoftObjectPath> CandidatePaths;
		CandidatePaths.Reserve(4);

		CandidatePaths.Add(MakeObjectPath(FString::Printf(TEXT("%s/%s"), DBAPaths::DT_FixedSkillGroups, *AssetName), AssetName));
		CandidatePaths.Add(MakeObjectPath(FString::Printf(TEXT("/Game/DBA/Data/SkillGroups/%s"), *AssetName), AssetName));
		CandidatePaths.Add(MakeObjectPath(FString::Printf(TEXT("/Game/DBA/Data/SkillGroups/DA_FSG_%s"), *AssetName), FString::Printf(TEXT("DA_FSG_%s"), *AssetName)));

		const FString StandardAssetName = FString::Printf(TEXT("DA_FSG_Standard_5v5_%d"), GetStandardSkillGroupIndex(AssetName));
		CandidatePaths.Add(MakeObjectPath(FString::Printf(TEXT("/Game/DBA/Data/SkillGroups/%s"), *StandardAssetName), StandardAssetName));

		return CandidatePaths;
	}

	UDBAFixedSkillGroupDataAsset* ResolveLoadedFixedSkillGroupAsset(const FSoftObjectPath& AssetPath)
	{
		if (!DoesObjectPackageExist(AssetPath))
		{
			return nullptr;
		}

		return Cast<UDBAFixedSkillGroupDataAsset>(AssetPath.ResolveObject());
	}

	bool FindExistingFixedSkillGroupPath(const FName& FixedSkillGroupId, FSoftObjectPath& OutAssetPath)
	{
		for (const FSoftObjectPath& CandidatePath : MakeFixedSkillGroupCandidatePaths(FixedSkillGroupId))
		{
			if (DoesObjectPackageExist(CandidatePath))
			{
				OutAssetPath = CandidatePath;
				return true;
			}
		}
		return false;
	}

	TMap<FName, TStrongObjectPtr<UDBAFixedSkillGroupDataAsset>>& GetFallbackSkillGroupCache()
	{
		static TMap<FName, TStrongObjectPtr<UDBAFixedSkillGroupDataAsset>> Cache;
		return Cache;
	}

	UDBAFixedSkillGroupDataAsset* GetOrCreateFallbackFixedSkillGroup(const FName& FixedSkillGroupId)
	{
		TMap<FName, TStrongObjectPtr<UDBAFixedSkillGroupDataAsset>>& Cache = GetFallbackSkillGroupCache();
		if (TStrongObjectPtr<UDBAFixedSkillGroupDataAsset>* Existing = Cache.Find(FixedSkillGroupId))
		{
			return Existing->Get();
		}

		UDBAFixedSkillGroupDataAsset* FallbackAsset = NewObject<UDBAFixedSkillGroupDataAsset>(GetTransientPackage(), NAME_None, RF_Transient);
		FallbackAsset->FixedSkillGroupId = FixedSkillGroupId;
		FallbackAsset->PassiveAbilityClass = UDBAZodiacPassiveAbility_Generic::StaticClass();
		FallbackAsset->Skill01Class = UDBAElementSkillAbility_Generic::StaticClass();
		FallbackAsset->Skill02Class = UDBAElementSkillAbility_Generic::StaticClass();
		FallbackAsset->Skill03Class = UDBAElementSkillAbility_Generic::StaticClass();
		FallbackAsset->Skill04Class = UDBAElementSkillAbility_Generic::StaticClass();
		FallbackAsset->ZodiacUltimateClass = UDBAZodiacUltimateAbility_Generic::StaticClass();

		Cache.Add(FixedSkillGroupId, TStrongObjectPtr<UDBAFixedSkillGroupDataAsset>(FallbackAsset));
		UE_LOG(LogDBAData, Verbose, TEXT("[UDBAFixedSkillGroupLibrary] \u4f7f\u7528\u8fd0\u884c\u65f6\u515c\u5e95\u6280\u80fd\u7ec4\uff1a%s"), *FixedSkillGroupId.ToString());
		return FallbackAsset;
	}
}

UDBAFixedSkillGroupDataAsset* UDBAFixedSkillGroupLibrary::GetFixedSkillGroupById(const FName& FixedSkillGroupId)
{
	if (FixedSkillGroupId.IsNone())
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] \u56fa\u5b9a\u6280\u80fd\u7ec4 ID \u4e3a\u7a7a\u3002"));
		return nullptr;
	}

	for (const FSoftObjectPath& AssetPath : MakeFixedSkillGroupCandidatePaths(FixedSkillGroupId))
	{
		if (UDBAFixedSkillGroupDataAsset* LoadedAsset = ResolveLoadedFixedSkillGroupAsset(AssetPath))
		{
			return LoadedAsset;
		}
		if (DoesObjectPackageExist(AssetPath))
		{
			if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
			{
				AssetManager->GetStreamableManager().RequestAsyncLoad(
					TArray<FSoftObjectPath>({ AssetPath }),
					FStreamableDelegate(),
					FStreamableManager::AsyncLoadHighPriority,
					true);
			}
			break;
		}
	}

	return GetOrCreateFallbackFixedSkillGroup(FixedSkillGroupId);
}

void UDBAFixedSkillGroupLibrary::LoadFixedSkillGroupByIdAsync(const FName& FixedSkillGroupId, FDBAOnFixedSkillGroupLoaded OnLoadedDelegate)
{
	if (FixedSkillGroupId.IsNone())
	{
		UE_LOG(LogDBAData, Warning, TEXT("[UDBAFixedSkillGroupLibrary] \u56fa\u5b9a\u6280\u80fd\u7ec4 ID \u4e3a\u7a7a\u3002"));
		OnLoadedDelegate.ExecuteIfBound(nullptr);
		return;
	}

	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager)
	{
		OnLoadedDelegate.ExecuteIfBound(GetOrCreateFallbackFixedSkillGroup(FixedSkillGroupId));
		return;
	}

	FSoftObjectPath AssetPath;
	if (!FindExistingFixedSkillGroupPath(FixedSkillGroupId, AssetPath))
	{
		OnLoadedDelegate.ExecuteIfBound(GetOrCreateFallbackFixedSkillGroup(FixedSkillGroupId));
		return;
	}

	AssetManager->GetStreamableManager().RequestAsyncLoad(
		TArray<FSoftObjectPath>({ AssetPath }),
		[AssetPath, FixedSkillGroupId, OnLoadedDelegate]()
		{
			UObject* LoadedObject = AssetPath.ResolveObject();
			UDBAFixedSkillGroupDataAsset* LoadedAsset = Cast<UDBAFixedSkillGroupDataAsset>(LoadedObject);
			if (!LoadedAsset)
			{
				LoadedAsset = GetOrCreateFallbackFixedSkillGroup(FixedSkillGroupId);
			}
			OnLoadedDelegate.ExecuteIfBound(LoadedAsset);
		},
		FStreamableManager::AsyncLoadHighPriority,
		true);
}
