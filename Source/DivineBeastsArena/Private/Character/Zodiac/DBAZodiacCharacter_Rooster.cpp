// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 曜鸣神鸡

#include "Character/Zodiac/DBAZodiacCharacter_Rooster.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Rooster::ADBAZodiacCharacter_Rooster()
{
	// 设置元素类型
	ElementType = FName(TEXT("Water"));
	ZodiacType = FName(TEXT("Rooster"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Rooster/SK_Rooster_Mesh.SK_Rooster_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Rooster/ABP_Rooster.ABP_Rooster"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Rooster::BeginPlay()
{
	Super::BeginPlay();

	// 曜鸣神鸡角色特定初始化
}
