// Copyright Freebooz Games, Inc. All Rights Reserved.
// 守卫模型基类

#include "Character/Guardian/DBAGuardianBase.h"
#include "Components/StaticMeshComponent.h"

ADBAGuardianBase::ADBAGuardianBase()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ADBAGuardianBase::BeginPlay()
{
	Super::BeginPlay();
}

void ADBAGuardianBase::PlayAttackVFX(AActor* Target)
{
	// Attack VFX logic
}

void ADBAGuardianBase::PlayHitVFX(AActor* Attacker)
{
	// Hit VFX logic
}