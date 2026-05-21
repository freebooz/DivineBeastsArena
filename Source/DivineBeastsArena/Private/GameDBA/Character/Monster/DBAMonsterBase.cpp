// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物模型基类

#include "GameDBA/Character/Monster/DBAMonsterBase.h"
#include "GameDBA/Combat/Feedback/DBAFloatingDamageComponent.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ADBAMonsterBase::ADBAMonsterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
}

void ADBAMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentHealth = MaxHealth;
	}
}

void ADBAMonsterBase::OnRep_CurrentHealth()
{
}

float ADBAMonsterBase::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
}

float ADBAMonsterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float FinalDamage = AppliedDamage > 0.0f ? AppliedDamage : DamageAmount;
	if (!HasAuthority() || FinalDamage <= 0.0f || CurrentHealth <= 0.0f)
	{
		return FinalDamage;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.0f, MaxHealth);
	OnRep_CurrentHealth();
	PlayHitVFX(DamageCauser);
	MulticastShowDamageNumber(FinalDamage, GetActorLocation() + FVector(0.0f, 0.0f, 118.0f), false);

	UE_LOG(LogDBACombat, Log, TEXT("[DBAMonsterBase] 怪物受到伤害：怪物=%s 类型=%s 伤害=%.1f 生命=%.1f/%.1f 伤害来源=%s"),
		*GetName(),
		*MonsterType.ToString(),
		FinalDamage,
		CurrentHealth,
		MaxHealth,
		*GetNameSafe(DamageCauser));

	if (CurrentHealth <= 0.0f)
	{
		PlayDeathVFX();
		UE_LOG(LogDBACombat, Log, TEXT("[DBAMonsterBase] 怪物已被击败：怪物=%s 类型=%s"), *GetName(), *MonsterType.ToString());
		SetLifeSpan(1.0f);
		SetActorEnableCollision(false);
		if (GetMesh())
		{
			GetMesh()->SetHiddenInGame(true);
		}
	}

	return FinalDamage;
}

void ADBAMonsterBase::MulticastShowDamageNumber_Implementation(float DamageAmount, FVector_NetQuantize ImpactPoint, bool bIsCritical)
{
	if (UDBAFloatingDamageComponent* DamageComponent = FindComponentByClass<UDBAFloatingDamageComponent>())
	{
		DamageComponent->SpawnDamageNumber(DamageAmount, bIsCritical, 4, FVector(ImpactPoint));
	}
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
	static const FSoftObjectPath HitVFXPath(TEXT("/Game/VFX/Impacts/Hit_Default.Hit_Default"));
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
	static const FSoftObjectPath DeathVFXPath(TEXT("/Game/VFX/Death/Death_Default.Death_Default"));
	TSoftObjectPtr<UParticleSystem> DeathVFX(DeathVFXPath);

	if (UParticleSystem* VFX = DeathVFX.LoadSynchronous())
	{
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, DeathLocation, Rotation, true);
	}
}

void ADBAMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADBAMonsterBase, CurrentHealth);
}
