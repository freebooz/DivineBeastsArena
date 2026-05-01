// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - 幻云灵猿

#include "Character/Zodiac/DBAZodiacCharacter_Monkey.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_Monkey::ADBAZodiacCharacter_Monkey()
{
	// 设置元素类型
	ElementType = FName(TEXT("Fire"));
	ZodiacType = FName(TEXT("Monkey"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Monkey/SK_Monkey_Mesh.SK_Monkey_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Monkey/ABP_Monkey.ABP_Monkey"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Monkey::BeginPlay()
{
	Super::BeginPlay();

	// 幻云灵猿角色特定初始化
}
