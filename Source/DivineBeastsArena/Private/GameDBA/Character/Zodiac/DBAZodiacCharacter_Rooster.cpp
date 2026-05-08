// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 鏇滈福绁為浮

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Rooster.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Rooster::ADBAZodiacCharacter_Rooster()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Water;
	ZodiacType = EDBAZodiacType::Rooster;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Rooster/SK_Rooster_Mesh.SK_Rooster_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Rooster/ABP_Rooster"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Rooster::BeginPlay()
{
	Super::BeginPlay();

	// 鏇滈福绁為浮瑙掕壊鐗瑰畾鍒濆鍖?
}


