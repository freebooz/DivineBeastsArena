// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 澶滈殣鐏甸紶

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Rat.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Rat::ADBAZodiacCharacter_Rat()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Fire;
	ZodiacType = EDBAZodiacType::Rat;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Rat/SK_Rat_Mesh.SK_Rat_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Rat/ABP_Rat"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Rat::BeginPlay()
{
	Super::BeginPlay();

	// 澶滈殣鐏甸紶瑙掕壊鐗瑰畾鍒濆鍖?
}


