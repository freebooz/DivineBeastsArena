// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DBAEditorAnimationTools.generated.h"

class USkeletalMesh;
class USkeleton;
class UAnimationAsset;

UCLASS()
class DIVINEBEASTSARENA_API UDBAEditorAnimationTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|Editor|Animation")
	static bool AssignSkeletonToMesh(USkeletalMesh* SkeletalMesh, USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "DBA|Editor|Animation")
	static bool AssignSkeletonToMeshByPath(const FString& SkeletalMeshObjectPath, const FString& SkeletonObjectPath);

	UFUNCTION(BlueprintCallable, Category = "DBA|Editor|Animation")
	static bool AssignSkeletonToAnimationAsset(UAnimationAsset* AnimationAsset, USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "DBA|Editor|Animation")
	static int32 AssignSkeletonToAnimationAssetsInPath(const FString& ContentPath, USkeleton* Skeleton, bool bRecursive = true);
};
