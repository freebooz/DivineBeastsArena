// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 夜隐灵鼠

#include "Character/Zodiac/DBAZodiacCharacter_Rat.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Rat::ADBAZodiacCharacter_Rat()
{
	// 设置元素类型
	ElementType = FName(TEXT("Fire"));
	ZodiacType = FName(TEXT("Rat"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Rat/SK_Rat_Mesh.SK_Rat_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Rat/ABP_Rat.ABP_Rat"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Rat::BeginPlay()
{
	Super::BeginPlay();

	// 夜隐灵鼠角色特定初始化
}
