// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 玄花灵蛇

#include "Character/Zodiac/DBAZodiacCharacter_Snake.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Snake::ADBAZodiacCharacter_Snake()
{
	// 设置元素类型
	ElementType = FName(TEXT("Water"));
	ZodiacType = FName(TEXT("Snake"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Snake/SK_Snake_Mesh.SK_Snake_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Snake/ABP_Snake.ABP_Snake"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Snake::BeginPlay()
{
	Super::BeginPlay();

	// 玄花灵蛇角色特定初始化
}
