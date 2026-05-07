// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 骞讳簯鐏电尶

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Monkey.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Monkey::ADBAZodiacCharacter_Monkey()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Fire;
	ZodiacType = EDBAZodiacType::Monkey;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Monkey/SK_Monkey_Mesh.SK_Monkey_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Monkey/ABP_Monkey.ABP_Monkey"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Monkey::BeginPlay()
{
	Super::BeginPlay();

	// 骞讳簯鐏电尶瑙掕壊鐗瑰畾鍒濆鍖?
}

