// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 瑁傞铏庡悰

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Tiger.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Tiger::ADBAZodiacCharacter_Tiger()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Wood;
	ZodiacType = EDBAZodiacType::Tiger;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Tiger/SK_Tiger_Mesh.SK_Tiger_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Tiger/ABP_Tiger.ABP_Tiger"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Tiger::BeginPlay()
{
	Super::BeginPlay();

	// 瑁傞铏庡悰瑙掕壊鐗瑰畾鍒濆鍖?
}

