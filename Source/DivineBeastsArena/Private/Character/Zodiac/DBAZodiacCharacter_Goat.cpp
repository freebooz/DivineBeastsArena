// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 灵泽仙羊

#include "Character/Zodiac/DBAZodiacCharacter_Goat.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Goat::ADBAZodiacCharacter_Goat()
{
	// 设置元素类型
	ElementType = FName(TEXT("Gold"));
	ZodiacType = FName(TEXT("Goat"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Goat/SK_Goat_Mesh.SK_Goat_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Goat/ABP_Goat.ABP_Goat"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Goat::BeginPlay()
{
	Super::BeginPlay();

	// 灵泽仙羊角色特定初始化
}
