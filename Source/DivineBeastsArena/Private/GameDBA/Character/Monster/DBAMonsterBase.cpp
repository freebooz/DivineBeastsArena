// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物模型基类

#include "GameDBA/Character/Monster/DBAMonsterBase.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Engine/World.h"

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
	if (!GetWorld())
	{
		return;
	}

	FVector HitLocation = GetActorLocation();

	// 根据攻击者类型播放不同受击特效
	// 这里使用默认的受击特效蓝本，实际项目中可以通过配置或DataTable指定
	static const FName HitVFXPath(TEXT("/Game/VFX/Impacts/Hit_Default.Hit_Default"));
	TSoftObjectPtr<UParticleSystem> HitVFX(HitVFXPath);

	if (UParticleSystem* VFX = HitVFX.LoadSynchronous())
	{
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, HitLocation, Rotation, true);
	}
}

void ADBAMonsterBase::PlayDeathVFX()
{
	if (!GetWorld())
	{
		return;
	}

	FVector DeathLocation = GetActorLocation();

	// 播放死亡特效
	static const FName DeathVFXPath(TEXT("/Game/VFX/Death/Death_Default.Death_Default"));
	TSoftObjectPtr<UParticleSystem> DeathVFX(DeathVFXPath);

	if (UParticleSystem* VFX = DeathVFX.LoadSynchronous())
	{
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, DeathLocation, Rotation, true);
	}
}