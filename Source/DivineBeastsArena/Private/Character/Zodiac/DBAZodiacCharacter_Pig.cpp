// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 福岳灵猪

#include "Character/Zodiac/DBAZodiacCharacter_Pig.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Pig::ADBAZodiacCharacter_Pig()
{
	// 设置元素类型
	ElementType = FName(TEXT("Gold"));
	ZodiacType = FName(TEXT("Pig"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Pig/SK_Pig_Mesh.SK_Pig_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Pig/ABP_Pig.ABP_Pig"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Pig::BeginPlay()
{
	Super::BeginPlay();

	// 福岳灵猪角色特定初始化
}
