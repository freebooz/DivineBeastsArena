// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Animation/DBAEditorAnimationTools.h"

#include "Animation/AnimationAsset.h"
#include "Animation/Skeleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/SkeletalMesh.h"

bool UDBAEditorAnimationTools::AssignSkeletonToMesh(USkeletalMesh* SkeletalMesh, USkeleton* Skeleton)
{
#if WITH_EDITOR
	if (!SkeletalMesh || !Skeleton)
	{
		return false;
	}

	SkeletalMesh->Modify();
	SkeletalMesh->SetSkeleton(Skeleton);
	SkeletalMesh->MarkPackageDirty();
	SkeletalMesh->PostEditChange();

	return SkeletalMesh->GetSkeleton() == Skeleton;
#else
	return false;
#endif
}

bool UDBAEditorAnimationTools::AssignSkeletonToMeshByPath(const FString& SkeletalMeshObjectPath, const FString& SkeletonObjectPath)
{
#if WITH_EDITOR
	USkeletalMesh* SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, *SkeletalMeshObjectPath);
	USkeleton* Skeleton = LoadObject<USkeleton>(nullptr, *SkeletonObjectPath);
	return AssignSkeletonToMesh(SkeletalMesh, Skeleton);
#else
	return false;
#endif
}

bool UDBAEditorAnimationTools::AssignSkeletonToAnimationAsset(UAnimationAsset* AnimationAsset, USkeleton* Skeleton)
{
#if WITH_EDITOR
	if (!AnimationAsset || !Skeleton)
	{
		return false;
	}

	AnimationAsset->Modify();
	AnimationAsset->SetSkeleton(Skeleton);
	AnimationAsset->MarkPackageDirty();
	AnimationAsset->PostEditChange();
	return AnimationAsset->GetSkeleton() == Skeleton;
#else
	return false;
#endif
}

int32 UDBAEditorAnimationTools::AssignSkeletonToAnimationAssetsInPath(const FString& ContentPath, USkeleton* Skeleton, bool bRecursive)
{
#if WITH_EDITOR
	if (!Skeleton || ContentPath.IsEmpty())
	{
		return 0;
	}

	int32 UpdatedCount = 0;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	FARFilter Filter;
	Filter.PackagePaths.Add(*ContentPath);
	Filter.bRecursivePaths = bRecursive;
	Filter.bRecursiveClasses = true;
	Filter.ClassPaths.Add(UAnimationAsset::StaticClass()->GetClassPathName());

	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetDataList);
	for (const FAssetData& AssetData : AssetDataList)
	{
		UObject* Loaded = AssetData.GetAsset();
		UAnimationAsset* AnimationAsset = Cast<UAnimationAsset>(Loaded);
		if (!AnimationAsset)
		{
			continue;
		}

		if (AssignSkeletonToAnimationAsset(AnimationAsset, Skeleton))
		{
			++UpdatedCount;
		}
	}

	return UpdatedCount;
#else
	return 0;
#endif
}
