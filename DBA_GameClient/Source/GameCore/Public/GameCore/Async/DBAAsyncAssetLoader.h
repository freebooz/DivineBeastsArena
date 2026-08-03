// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "UObject/SoftObjectPtr.h"

namespace DBAAsyncAssetLoader
{
	template <typename AssetType>
	void RequestAsyncAsset(
		UObject* CallbackOwner,
		TSoftObjectPtr<AssetType> Asset,
		TFunction<void(AssetType*)> OnLoaded)
	{
		if (Asset.IsNull() || !CallbackOwner)
		{
			return;
		}

		if (AssetType* LoadedAsset = Asset.Get())
		{
			OnLoaded(LoadedAsset);
			return;
		}

		UAssetManager::GetStreamableManager().RequestAsyncLoad(
			Asset.ToSoftObjectPath(),
			FStreamableDelegate::CreateWeakLambda(CallbackOwner, [Asset, OnLoaded = MoveTemp(OnLoaded)]() mutable
			{
				if (AssetType* LoadedAsset = Asset.Get())
				{
					OnLoaded(LoadedAsset);
				}
			}),
			FStreamableManager::AsyncLoadHighPriority,
			true);
	}

	template <typename AssetType>
	void AddPreloadPath(const TSoftObjectPtr<AssetType>& Asset, TArray<FSoftObjectPath>& OutPaths)
	{
		if (!Asset.IsNull() && !Asset.Get())
		{
			OutPaths.AddUnique(Asset.ToSoftObjectPath());
		}
	}

	inline void RequestAsyncPreload(UObject* CallbackOwner, const TArray<FSoftObjectPath>& Paths)
	{
		if (!CallbackOwner || Paths.IsEmpty())
		{
			return;
		}

		UAssetManager::GetStreamableManager().RequestAsyncLoad(
			Paths,
			FStreamableDelegate(),
			FStreamableManager::AsyncLoadHighPriority,
			true);
	}
}
