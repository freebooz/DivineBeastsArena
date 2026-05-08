// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 鐏垫辰浠欑緤

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Goat.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Goat::ADBAZodiacCharacter_Goat()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Metal;
	ZodiacType = EDBAZodiacType::Goat;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Goat/SK_Goat_Mesh.SK_Goat_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Goat/ABP_Goat"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Goat::BeginPlay()
{
	Super::BeginPlay();

	// 鐏垫辰浠欑緤瑙掕壊鐗瑰畾鍒濆鍖?
}


