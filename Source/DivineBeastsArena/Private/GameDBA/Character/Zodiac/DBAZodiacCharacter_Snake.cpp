// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 鐜勮姳鐏佃泧

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Snake.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Snake::ADBAZodiacCharacter_Snake()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Water;
	ZodiacType = EDBAZodiacType::Snake;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Snake/SK_Snake_Mesh.SK_Snake_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Snake/ABP_Snake"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Snake::BeginPlay()
{
	Super::BeginPlay();

	// 鐜勮姳鐏佃泧瑙掕壊鐗瑰畾鍒濆鍖?
}


