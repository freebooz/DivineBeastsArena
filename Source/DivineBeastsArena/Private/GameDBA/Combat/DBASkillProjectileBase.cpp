// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/DBASkillProjectileBase.h"

#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"

namespace
{
	FString SoftObjectPathString(const FSoftObjectPath& Path)
	{
		return Path.IsValid() ? Path.ToString() : FString();
	}
}

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

	ProjectileLoopAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ProjectileLoopAudio"));
	ProjectileLoopAudio->SetupAttachment(RootComponent);
	ProjectileLoopAudio->bAutoActivate = false;
	ProjectileLoopAudio->bAllowSpatialization = true;
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

	const FString ProjectileVFXPath = SoftObjectPathString(ProjectileVFXAsset.ToSoftObjectPath());
	const FString ProjectileNiagaraVFXPath = SoftObjectPathString(ProjectileNiagaraVFXAsset.ToSoftObjectPath());
	const FString FlySFXPath = SoftObjectPathString(FlySFXAsset.ToSoftObjectPath());
	ApplyProjectileVisualsLocal(ProjectileVFXPath, ProjectileNiagaraVFXPath, FlySFXPath);
	if (HasAuthority() && GetNetMode() != NM_Standalone)
	{
		MulticastApplyProjectileVisuals(ProjectileVFXPath, ProjectileNiagaraVFXPath, FlySFXPath);
	}

	if (InTarget)
	{
		LaunchProjectile(InTarget->GetActorLocation() - GetActorLocation());
	}
}

void ADBASkillProjectileBase::MulticastApplyProjectileVisuals_Implementation(
	const FString& ProjectileVFXPath,
	const FString& ProjectileNiagaraVFXPath,
	const FString& FlySFXPath)
{
	if (HasAuthority())
	{
		return;
	}

	ApplyProjectileVisualsLocal(ProjectileVFXPath, ProjectileNiagaraVFXPath, FlySFXPath);
}

void ADBASkillProjectileBase::ApplyProjectileVisualsLocal(
	const FString& ProjectileVFXPath,
	const FString& ProjectileNiagaraVFXPath,
	const FString& FlySFXPath)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!ProjectileVFXPath.IsEmpty())
	{
		if (UParticleSystem* VFX = LoadObject<UParticleSystem>(nullptr, *ProjectileVFXPath))
		{
			ProjectileVFX->SetTemplate(VFX);
			ProjectileVFX->Activate(true);
		}
		else
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBASkillProjectileBase] 加载投射物 Cascade 飞行特效失败：技能=%s 路径=%s"),
				*SkillId.ToString(),
				*ProjectileVFXPath);
		}
	}

	if (!ProjectileNiagaraVFXPath.IsEmpty())
	{
		if (UNiagaraSystem* VFX = LoadObject<UNiagaraSystem>(nullptr, *ProjectileNiagaraVFXPath))
		{
			ProjectileNiagaraVFX->SetAsset(VFX);
			ProjectileNiagaraVFX->SetVisibility(true);
			ProjectileNiagaraVFX->Activate(true);
		}
		else
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBASkillProjectileBase] 加载投射物 Niagara 飞行特效失败：技能=%s 路径=%s"),
				*SkillId.ToString(),
				*ProjectileNiagaraVFXPath);
		}
	}

	if (!FlySFXPath.IsEmpty() && ProjectileLoopAudio)
	{
		if (USoundBase* FlySFX = LoadObject<USoundBase>(nullptr, *FlySFXPath))
		{
			ProjectileLoopAudio->SetSound(FlySFX);
			ProjectileLoopAudio->Play();
		}
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
	const FString ImpactVFXPath = SoftObjectPathString(ImpactVFXAsset.ToSoftObjectPath());
	const FString ImpactNiagaraVFXPath = SoftObjectPathString(ImpactNiagaraVFXAsset.ToSoftObjectPath());
	const FString ImpactSFXPath = SoftObjectPathString(ImpactSFXAsset.ToSoftObjectPath());
	PlayImpactFeedbackLocal(ImpactVFXPath, ImpactNiagaraVFXPath, ImpactSFXPath, HitLocation, GetActorRotation());
	if (HasAuthority() && GetNetMode() != NM_Standalone)
	{
		MulticastPlayImpactFeedback(ImpactVFXPath, ImpactNiagaraVFXPath, ImpactSFXPath, HitLocation, GetActorRotation());
	}

	if (HitActor && HitActor != ProjectileOwner && Damage > 0.0f)
	{
		FPointDamageEvent DamageEvent;
		DamageEvent.Damage = Damage;
		DamageEvent.HitInfo = FHitResult(HitActor, nullptr, HitLocation, FVector::ZeroVector);
		HitActor->TakeDamage(Damage, DamageEvent, ProjectileOwner ? ProjectileOwner->GetInstigatorController() : nullptr, ProjectileOwner);
	}

	if (ProjectileLoopAudio)
	{
		ProjectileLoopAudio->Stop();
	}

	BP_OnProjectileHit(HitActor, HitLocation);
	Destroy();
}

void ADBASkillProjectileBase::MulticastPlayImpactFeedback_Implementation(
	const FString& ImpactVFXPath,
	const FString& ImpactNiagaraVFXPath,
	const FString& ImpactSFXPath,
	FVector_NetQuantize HitLocation,
	FRotator HitRotation)
{
	if (HasAuthority())
	{
		return;
	}

	PlayImpactFeedbackLocal(ImpactVFXPath, ImpactNiagaraVFXPath, ImpactSFXPath, HitLocation, HitRotation);
}

void ADBASkillProjectileBase::PlayImpactFeedbackLocal(
	const FString& ImpactVFXPath,
	const FString& ImpactNiagaraVFXPath,
	const FString& ImpactSFXPath,
	const FVector& HitLocation,
	const FRotator& HitRotation)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!ImpactNiagaraVFXPath.IsEmpty())
	{
		if (UNiagaraSystem* VFX = LoadObject<UNiagaraSystem>(nullptr, *ImpactNiagaraVFXPath))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				VFX,
				HitLocation,
				HitRotation,
				FVector(1.0f),
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true);
		}
	}

	if (!ImpactVFXPath.IsEmpty())
	{
		if (UParticleSystem* VFX = LoadObject<UParticleSystem>(nullptr, *ImpactVFXPath))
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, HitLocation, FRotator::ZeroRotator, true);
		}
	}

	if (!ImpactSFXPath.IsEmpty())
	{
		if (USoundBase* SFX = LoadObject<USoundBase>(nullptr, *ImpactSFXPath))
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, HitLocation);
		}
	}
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
	if (!HasAuthority())
	{
		return;
	}

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
	if (!HasAuthority())
	{
		return;
	}

	if (OtherActor && OtherActor != this && OtherActor != ProjectileOwner)
	{
		const FVector ImpactPoint = SweepResult.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(SweepResult.ImpactPoint);
		OnProjectileHit(OtherActor, ImpactPoint);
	}
}
