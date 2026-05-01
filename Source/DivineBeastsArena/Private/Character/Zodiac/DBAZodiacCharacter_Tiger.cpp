// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 裂风虎君

#include "Character/Zodiac/DBAZodiacCharacter_Tiger.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Tiger::ADBAZodiacCharacter_Tiger()
{
	// 设置元素类型
	ElementType = FName(TEXT("Wood"));
	ZodiacType = FName(TEXT("Tiger"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Tiger/SK_Tiger_Mesh.SK_Tiger_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Tiger/ABP_Tiger.ABP_Tiger"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Tiger::BeginPlay()
{
	Super::BeginPlay();

	// 裂风虎君角色特定初始化
}
