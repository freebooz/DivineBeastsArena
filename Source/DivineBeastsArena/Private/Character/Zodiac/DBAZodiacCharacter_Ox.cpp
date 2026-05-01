// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 镇岳神牛

#include "Character/Zodiac/DBAZodiacCharacter_Ox.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Ox::ADBAZodiacCharacter_Ox()
{
	// 设置元素类型
	ElementType = FName(TEXT("Water"));
	ZodiacType = FName(TEXT("Ox"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Ox/SK_Ox_Mesh.SK_Ox_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Ox/ABP_Ox.ABP_Ox"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Ox::BeginPlay()
{
	Super::BeginPlay();

	// 镇岳神牛角色特定初始化
}
