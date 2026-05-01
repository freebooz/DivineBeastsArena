// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 镇魄灵犬

#include "Character/Zodiac/DBAZodiacCharacter_Dog.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Dog::ADBAZodiacCharacter_Dog()
{
	// 设置元素类型
	ElementType = FName(TEXT("Wood"));
	ZodiacType = FName(TEXT("Dog"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Dog/SK_Dog_Mesh.SK_Dog_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Dog/ABP_Dog.ABP_Dog"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Dog::BeginPlay()
{
	Super::BeginPlay();

	// 镇魄灵犬角色特定初始化
}
