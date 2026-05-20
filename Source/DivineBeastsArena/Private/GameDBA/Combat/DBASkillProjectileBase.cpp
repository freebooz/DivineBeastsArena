// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/DBASkillProjectileBase.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

ADBASkillProjectileBase::ADBASkillProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	InitialLifeSpan = 5.0f;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionSphere->InitSphereRadius(Radius);
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->OnComponentHit.AddDynamic(this, &ADBASkillProjectileBase::HandleProjectileHit);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADBASkillProjectileBase::HandleProjectileOverlap);
	RootComponent = CollisionSphere;

	UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetHiddenInGame(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed * 1.5f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bInitialVelocityInLocalSpace = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	ProjectileVFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ProjectileVFX"));
	ProjectileVFX->SetupAttachment(RootComponent);
	ProjectileVFX->bAutoActivate = true;

	ProjectileNiagaraVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraVFX"));
	ProjectileNiagaraVFX->SetupAttachment(RootComponent);
	ProjectileNiagaraVFX->bAutoActivate = true;
}

void ADBASkillProjectileBase::InitializeProjectile(
	FName InSkillId,
	AActor* InOwner,
	AActor* InTarget,
	float InDamage,
	float InSpeed,
	float InRadius)
{
	SkillId = InSkillId;
	ProjectileOwner = InOwner;
	TargetActor = InTarget;
	SetProjectileProperties(InSpeed, InRadius, InDamage);

	if (CollisionSphere && InOwner)
	{
		CollisionSphere->IgnoreActorWhenMoving(InOwner, true);
	}

	if (ProjectileVFXAsset.IsValid())
	{
		if (UParticleSystem* VFX = ProjectileVFXAsset.LoadSynchronous())
		{
			ProjectileVFX->SetTemplate(VFX);
			ProjectileVFX->Activate(true);
		}
	}

	if (!ProjectileNiagaraVFXAsset.IsNull())
	{
		if (UNiagaraSystem* VFX = ProjectileNiagaraVFXAsset.LoadSynchronous())
		{
			ProjectileNiagaraVFX->SetAsset(VFX);
			ProjectileNiagaraVFX->Activate(true);
		}
	}

	if (InTarget)
	{
		LaunchProjectile(InTarget->GetActorLocation() - GetActorLocation());
	}
}

void ADBASkillProjectileBase::SetProjectileProperties(float InSpeed, float InRadius, float InDamage)
{
	Speed = InSpeed;
	Radius = InRadius;
	Damage = InDamage;

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed * 1.5f;
	}

	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(Radius);
	}
}

void ADBASkillProjectileBase::LaunchProjectile(const FVector& Direction)
{
	if (!ProjectileMovement)
	{
		return;
	}

	const FVector SafeDirection = Direction.GetSafeNormal();
	if (SafeDirection.IsNearlyZero())
	{
		return;
	}

	ProjectileMovement->Velocity = SafeDirection * Speed;
	SetActorRotation(SafeDirection.Rotation());
}

void ADBASkillProjectileBase::OnProjectileHit(AActor* HitActor, FVector HitLocation)
{
	if (!ImpactNiagaraVFXAsset.IsNull())
	{
		if (UNiagaraSystem* VFX = ImpactNiagaraVFXAsset.LoadSynchronous())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				VFX,
				HitLocation,
				GetActorRotation(),
				FVector(1.0f),
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true);
		}
	}

	if (ImpactVFXAsset.IsValid())
	{
		if (UParticleSystem* VFX = ImpactVFXAsset.LoadSynchronous())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, HitLocation, FRotator::ZeroRotator, true);
		}
	}

	if (ImpactSFXAsset.IsValid())
	{
		if (USoundBase* SFX = ImpactSFXAsset.LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, HitLocation);
		}
	}

	if (HitActor && HitActor != ProjectileOwner && Damage > 0.0f)
	{
		FPointDamageEvent DamageEvent;
		DamageEvent.Damage = Damage;
		DamageEvent.HitInfo = FHitResult(HitActor, nullptr, HitLocation, FVector::ZeroVector);
		HitActor->TakeDamage(Damage, DamageEvent, ProjectileOwner ? ProjectileOwner->GetInstigatorController() : nullptr, ProjectileOwner);
	}

	BP_OnProjectileHit(HitActor, HitLocation);
	Destroy();
}

void ADBASkillProjectileBase::SetCollisionChannel(ECollisionChannel Channel)
{
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionObjectType(Channel);
	}
}

void ADBASkillProjectileBase::HandleProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this && OtherActor != ProjectileOwner)
	{
		const FVector ImpactPoint = Hit.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
		OnProjectileHit(OtherActor, ImpactPoint);
	}
}

void ADBASkillProjectileBase::HandleProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor != ProjectileOwner)
	{
		const FVector ImpactPoint = SweepResult.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(SweepResult.ImpactPoint);
		OnProjectileHit(OtherActor, ImpactPoint);
	}
}
