// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 绂忓渤鐏电尓

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Pig.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Pig::ADBAZodiacCharacter_Pig()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Metal;
	ZodiacType = EDBAZodiacType::Pig;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Pig/SK_Pig_Mesh.SK_Pig_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Pig/ABP_Pig"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Pig::BeginPlay()
{
	Super::BeginPlay();

	// 绂忓渤鐏电尓瑙掕壊鐗瑰畾鍒濆鍖?
}


