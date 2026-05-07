// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鎶€鑳芥姇灏勭墿 - 闀囬瓌鐏电姮鎶€鑳紼

#include "GameDBA/Combat/DBAProjectile_Dog_E.h"
#include "Components/SphereComponent.h"

ADBAProjectile_Dog_E::ADBAProjectile_Dog_E()
{
	// 璁剧疆鎶曞皠鐗╁睘鎬?	Speed = 1200.0f;
	Radius = 30.0f;
	Damage = 50.0f;

	if (!IsRunningDedicatedServer())
	{
		// 璁剧疆鐗规晥璧勬簮璺緞
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ProjectileVFXFinder(
		TEXT("/Game/VFX/Projectiles/Dog/P_Dog_E_Projectile.P_Dog_E_Projectile"));
	if (ProjectileVFXFinder.Succeeded())
	{
		ProjectileVFXAsset = ProjectileVFXFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ImpactVFXFinder(
		TEXT("/Game/VFX/Projectiles/Dog/P_Dog_E_Impact.P_Dog_E_Impact"));
	if (ImpactVFXFinder.Succeeded())
	{
		ImpactVFXAsset = ImpactVFXFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> FlySFXFinder(
		TEXT("/Game/Audio/SFX/Projectiles/Dog/S_Dog_E_Fly.S_Dog_E_Fly"));
	if (FlySFXFinder.Succeeded())
	{
		FlySFXAsset = FlySFXFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> ImpactSFXFinder(
		TEXT("/Game/Audio/SFX/Projectiles/Dog/S_Dog_E_Impact.S_Dog_E_Impact"));
	if (ImpactSFXFinder.Succeeded())
	{
		ImpactSFXAsset = ImpactSFXFinder.Object;
	}
	}
}

void ADBAProjectile_Dog_E::BeginPlay()
{
	Super::BeginPlay();
}

void ADBAProjectile_Dog_E::InitializeProjectile(
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
	Damage = InDamage;
	Speed = InSpeed;
	Radius = InRadius;

	// 鏇存柊绉诲姩缁勪欢
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed * 1.5f;
	}

	// 鏇存柊纰版挒鍗婂緞
	if (USphereComponent* Sphere = Cast<USphereComponent>(RootComponent))
	{
		Sphere->SetSphereRadius(Radius);
	}

	// 鍔犺浇椋炶鐗规晥
	if (ProjectileVFXAsset.IsValid())
	{
		if (UParticleSystem* VFX = ProjectileVFXAsset.LoadSynchronous())
		{
			ProjectileVFX->SetTemplate(VFX);
		}
	}

	// 璁剧疆鍒濆閫熷害鏂瑰悜鏈濆悜鐩爣
	if (InTarget)
	{
		FVector Direction = (InTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		ProjectileMovement->Velocity = Direction * Speed;
	}
}


