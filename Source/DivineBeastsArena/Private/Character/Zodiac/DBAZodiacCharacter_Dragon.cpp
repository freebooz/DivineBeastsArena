// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 云巡龙君

#include "Character/Zodiac/DBAZodiacCharacter_Dragon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Dragon::ADBAZodiacCharacter_Dragon()
{
	// 设置元素类型
	ElementType = FName(TEXT("Fire"));
	ZodiacType = FName(TEXT("Dragon"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Dragon/SK_Dragon_Mesh.SK_Dragon_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Dragon/ABP_Dragon.ABP_Dragon"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Dragon::BeginPlay()
{
	Super::BeginPlay();

	// 云巡龙君角色特定初始化
}
