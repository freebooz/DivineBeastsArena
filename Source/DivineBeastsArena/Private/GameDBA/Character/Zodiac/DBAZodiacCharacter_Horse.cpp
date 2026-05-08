// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 韪忛澶╅┕

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Horse.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Horse::ADBAZodiacCharacter_Horse()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Wood;
	ZodiacType = EDBAZodiacType::Horse;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Horse/SK_Horse_Mesh.SK_Horse_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Horse/ABP_Horse"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Horse::BeginPlay()
{
	Super::BeginPlay();

	// 韪忛澶╅┕瑙掕壊鐗瑰畾鍒濆鍖?
}


