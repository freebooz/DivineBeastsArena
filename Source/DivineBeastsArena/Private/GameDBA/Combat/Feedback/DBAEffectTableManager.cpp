// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/Feedback/DBAEffectTableManager.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameInstance.h"

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
	if (!TablePath.IsValid())
	{
		return;
	}

	// 同步加载
	UDataTable* LoadedTable = TablePath.LoadSynchronous();
	if (LoadedTable)
	{
		SkillEffectTable = LoadedTable;

		// 缓存所有数据
		CachedEffects.Empty();
		TArray<FName> RowNames = LoadedTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			if (FDBASkillEffectRow* Row = LoadedTable->FindRow<FDBASkillEffectRow>(RowName, TEXT("")))
			{
				CachedEffects.Add(RowName, *Row);
			}
		}
	}
}

TSoftObjectPtr<UDataTable> UDBAEffectTableManager::AsyncLoadSkillEffectTable(const TSoftObjectPtr<UDataTable>& TablePath)
{
	if (!TablePath.IsValid() || IsTableLoaded())
	{
		return TablePath;
	}

	PendingTablePath = TablePath;

	// 使用Asset Manager异步加载
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

FDBASkillEffectRow* UDBAEffectTableManager::GetSkillEffect(FName SkillID) const
{
	if (const FDBASkillEffectRow* Row = CachedEffects.Find(SkillID))
	{
		return const_cast<FDBASkillEffectRow*>(Row);
	}
	return nullptr;
}

TArray<FName> UDBAEffectTableManager::GetAllSkillIDs() const
{
	TArray<FName> Keys;
	CachedEffects.GetKeys(Keys);
	return Keys;
}

void UDBAEffectTableManager::OnAsyncLoadComplete(const TSoftObjectPtr<UDataTable>& TablePath)
{
	if (UDataTable* LoadedTable = TablePath.Get())
	{
		SkillEffectTable = LoadedTable;

		// 重新缓存
		CachedEffects.Empty();
		TArray<FName> RowNames = LoadedTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			if (FDBASkillEffectRow* Row = LoadedTable->FindRow<FDBASkillEffectRow>(RowName, TEXT("")))
			{
				CachedEffects.Add(RowName, *Row);
			}
		}
	}
}