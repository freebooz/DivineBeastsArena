// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 踏风天驹

#include "Character/Zodiac/DBAZodiacCharacter_Horse.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Horse::ADBAZodiacCharacter_Horse()
{
	// 设置元素类型
	ElementType = FName(TEXT("Wood"));
	ZodiacType = FName(TEXT("Horse"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Horse/SK_Horse_Mesh.SK_Horse_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Horse/ABP_Horse.ABP_Horse"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Horse::BeginPlay()
{
	Super::BeginPlay();

	// 踏风天驹角色特定初始化
}
