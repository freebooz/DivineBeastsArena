// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Combat/Feedback/DBAEffectTableManager.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "GameDBA/Utilities/DBAAsyncAssetLoader.h"

UDBAEffectTableManager::UDBAEffectTableManager()
{
}

void UDBAEffectTableManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UDBAEffectTableManager::Deinitialize()
{
	SkillEffectTable = nullptr;
	CachedEffects.Empty();
	Super::Deinitialize();
}

void UDBAEffectTableManager::LoadSkillEffectTable(const TSoftObjectPtr<UDataTable>& TablePath)
{
	AsyncLoadSkillEffectTable(TablePath);
}

TSoftObjectPtr<UDataTable> UDBAEffectTableManager::AsyncLoadSkillEffectTable(const TSoftObjectPtr<UDataTable>& TablePath)
{
	if (!TablePath.IsValid() || IsTableLoaded())
	{
		return TablePath;
	}

	PendingTablePath = TablePath;

	// 浣跨敤Asset Manager寮傛鍔犺浇
	if (UAssetManager* Manager = UAssetManager::GetIfInitialized())
	{
		TArray<FSoftObjectPath> Paths;
		Paths.Add(TablePath.ToSoftObjectPath());

		FStreamableDelegate Delegate;
		Delegate.BindUObject(this, &UDBAEffectTableManager::OnAsyncLoadComplete, TablePath);

		Manager->LoadAssetList(Paths, Delegate, FStreamableManager::AsyncLoadHighPriority);
	}

	return TablePath;
}

FDBASkillEffectRow UDBAEffectTableManager::GetSkillEffect(FName SkillID) const
{
	if (const FDBASkillEffectRow* Row = CachedEffects.Find(SkillID))
	{
		return *Row;
	}
	return FDBASkillEffectRow();
}

TArray<FName> UDBAEffectTableManager::GetAllSkillIDs() const
{
	TArray<FName> Keys;
	CachedEffects.GetKeys(Keys);
	return Keys;
}

void UDBAEffectTableManager::OnAsyncLoadComplete(TSoftObjectPtr<UDataTable> TablePath)
{
	if (UDataTable* LoadedTable = TablePath.Get())
	{
		SkillEffectTable = LoadedTable;

		CachedEffects.Empty();
		TArray<FSoftObjectPath> PresentationPaths;
		TArray<FName> RowNames = LoadedTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			if (FDBASkillEffectRow* Row = LoadedTable->FindRow<FDBASkillEffectRow>(RowName, TEXT("")))
			{
				CachedEffects.Add(RowName, *Row);
				DBAAsyncAssetLoader::AddPreloadPath(Row->ReleaseEffect, PresentationPaths);
				DBAAsyncAssetLoader::AddPreloadPath(Row->HitEffect, PresentationPaths);
				DBAAsyncAssetLoader::AddPreloadPath(Row->CastSound, PresentationPaths);
				DBAAsyncAssetLoader::AddPreloadPath(Row->HitSound, PresentationPaths);
			}
		}
		DBAAsyncAssetLoader::RequestAsyncPreload(this, PresentationPaths);
	}
}
