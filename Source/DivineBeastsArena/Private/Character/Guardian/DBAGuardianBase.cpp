// Copyright Freebooz Games, Inc. All Rights Reserved.
// 守卫模型基类

#include "Character/Guardian/DBAGuardianBase.h"
#include "DBA/VFX/Components/DBAZodiacVFXComponent.h"
#include "Components/StaticMeshComponent.h"

ADBAGuardianBase::ADBAGuardianBase()
{
	PrimaryActorTick.bCanEverTick = true;

	VFXComponent = CreateDefaultSubobject<UDBAZodiacVFXComponent>(TEXT("VFXComponent"));

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ADBAGuardianBase::BeginPlay()
{
	Super::BeginPlay();
}

void ADBAGuardianBase::PlayAttackVFX(AActor* Target)
{
	if (VFXComponent)
	{
		VFXComponent->PlayAttackVFX(Target);
	}
}

void ADBAGuardianBase::PlayHitVFX(AActor* Attacker)
{
	if (VFXComponent)
	{
		VFXComponent->PlayHitVFX(Attacker);
	}
}