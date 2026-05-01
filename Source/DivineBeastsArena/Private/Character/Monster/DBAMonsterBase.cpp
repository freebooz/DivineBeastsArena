// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物模型基类

#include "Character/Monster/DBAMonsterBase.h"
#include "Components/CapsuleComponent.h"

ADBAMonsterBase::ADBAMonsterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
}

void ADBAMonsterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ADBAMonsterBase::PlayHitVFX(AActor* Attacker)
{
	// VFX logic to be implemented based on attacker type
}

void ADBAMonsterBase::PlayDeathVFX()
{
	// Death VFX logic
}