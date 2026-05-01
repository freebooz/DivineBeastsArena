// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 月华灵兔

#include "Character/Zodiac/DBAZodiacCharacter_Rabbit.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Rabbit::ADBAZodiacCharacter_Rabbit()
{
	// 设置元素类型
	ElementType = FName(TEXT("Gold"));
	ZodiacType = FName(TEXT("Rabbit"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Rabbit/SK_Rabbit_Mesh.SK_Rabbit_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Rabbit/ABP_Rabbit.ABP_Rabbit"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Rabbit::BeginPlay()
{
	Super::BeginPlay();

	// 月华灵兔角色特定初始化
}
