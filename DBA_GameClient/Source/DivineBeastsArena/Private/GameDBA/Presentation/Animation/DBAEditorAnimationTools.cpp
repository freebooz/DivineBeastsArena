// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Presentation/Animation/DBAEditorAnimationTools.h"

#include "Animation/AnimationAsset.h"
#include "Animation/Skeleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/SkeletalMesh.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"

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

USkeleton* UDBAEditorAnimationTools::CreateOrUpdateSkeletonFromMesh(USkeletalMesh* SkeletalMesh, const FString& SkeletonPackagePath, const FString& SkeletonAssetName)
{
#if WITH_EDITOR
	if (!SkeletalMesh || SkeletonPackagePath.IsEmpty() || SkeletonAssetName.IsEmpty())
	{
		return nullptr;
	}

	const FString PackageName = SkeletonPackagePath / SkeletonAssetName;
	if (!FPackageName::IsValidLongPackageName(PackageName))
	{
		return nullptr;
	}

	const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *SkeletonAssetName);
	USkeleton* Skeleton = LoadObject<USkeleton>(nullptr, *ObjectPath);
	if (!Skeleton)
	{
		UPackage* Package = CreatePackage(*PackageName);
		if (!Package)
		{
			return nullptr;
		}

		Skeleton = NewObject<USkeleton>(Package, *SkeletonAssetName, RF_Public | RF_Standalone | RF_Transactional);
		if (!Skeleton)
		{
			return nullptr;
		}
		FAssetRegistryModule::AssetCreated(Skeleton);
	}

	Skeleton->Modify();
	if (!Skeleton->MergeAllBonesToBoneTree(SkeletalMesh, false))
	{
		return nullptr;
	}

	if (SkeletalMesh->GetSkeleton() != Skeleton)
	{
		SkeletalMesh->Modify();
		SkeletalMesh->SetSkeleton(Skeleton);
		SkeletalMesh->MarkPackageDirty();
		SkeletalMesh->PostEditChange();
	}

	Skeleton->MarkPackageDirty();
	Skeleton->PostEditChange();
	return Skeleton;
#else
	return nullptr;
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
