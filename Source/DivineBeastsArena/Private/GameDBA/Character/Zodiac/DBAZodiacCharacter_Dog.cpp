// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 闀囬瓌鐏电姮

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Dog.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Dog::ADBAZodiacCharacter_Dog()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Wood;
	ZodiacType = EDBAZodiacType::Dog;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Dog/SK_Dog_Mesh.SK_Dog_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Dog/ABP_Dog"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Dog::BeginPlay()
{
	Super::BeginPlay();

	// 闀囬瓌鐏电姮瑙掕壊鐗瑰畾鍒濆鍖?
}


