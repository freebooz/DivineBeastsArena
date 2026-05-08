// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 浜戝贰榫欏悰

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Dragon.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Dragon::ADBAZodiacCharacter_Dragon()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Fire;
	ZodiacType = EDBAZodiacType::Dragon;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Dragon/SK_Dragon_Mesh.SK_Dragon_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Dragon/ABP_Dragon"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Dragon::BeginPlay()
{
	Super::BeginPlay();

	// 浜戝贰榫欏悰瑙掕壊鐗瑰畾鍒濆鍖?
}


