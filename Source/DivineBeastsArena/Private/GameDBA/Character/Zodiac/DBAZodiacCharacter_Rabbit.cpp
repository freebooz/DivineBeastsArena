// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鐢熻倴瑙掕壊 - 鏈堝崕鐏靛厰

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Rabbit.h"
#include "Components/SkeletalMeshComponent.h"

ADBAZodiacCharacter_Rabbit::ADBAZodiacCharacter_Rabbit()
{
	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = EDBAElementType::Metal;
	ZodiacType = EDBAZodiacType::Rabbit;

	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/Models/Zodiac/Rabbit/SK_Rabbit_Mesh.SK_Rabbit_Mesh"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
		}

	// 璁剧疆鍔ㄧ敾钃濆浘
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("/Game/Animation/Zodiac/Rabbit/ABP_Rabbit.ABP_Rabbit"));
	if (AnimBPFinder.Succeeded())
	{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
		}

		}

	// 閰嶇疆瑙掕壊鎻忚堪
	PrimaryActorTick.bCanEverTick = true;
}

void ADBAZodiacCharacter_Rabbit::BeginPlay()
{
	Super::BeginPlay();

	// 鏈堝崕鐏靛厰瑙掕壊鐗瑰畾鍒濆鍖?
}

