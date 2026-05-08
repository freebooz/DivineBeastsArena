// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 闀囧渤绁炵墰

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Ox.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Ox::ADBAZodiacCharacter_Ox()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Water;
	ZodiacType = EDBAZodiacType::Ox;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Ox/SK_Ox_Mesh.SK_Ox_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Ox/ABP_Ox"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Ox::BeginPlay()
{
	Super::BeginPlay();

	// 闀囧渤绁炵墰瑙掕壊鐗瑰畾鍒濆鍖?
}


