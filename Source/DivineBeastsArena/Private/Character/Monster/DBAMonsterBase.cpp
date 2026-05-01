// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物模型基类

#include "Character/Monster/DBAMonsterBase.h"
#include "DBA/VFX/Components/DBAZodiacVFXComponent.h"
#include "Components/CapsuleComponent.h"

ADBAMonsterBase::ADBAMonsterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	VFXComponent = CreateDefaultSubobject<UDBAZodiacVFXComponent>(TEXT("VFXComponent"));

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
}

void ADBAMonsterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ADBAMonsterBase::PlayHitVFX(AActor* Attacker)
{
	if (VFXComponent)
	{
		VFXComponent->PlayHitVFX(Attacker);
	}
}

void ADBAMonsterBase::PlayDeathVFX()
{
	if (VFXComponent)
	{
		VFXComponent->PlayDeathVFX();
	}
}