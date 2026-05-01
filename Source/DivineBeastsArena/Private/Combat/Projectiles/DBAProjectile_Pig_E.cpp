// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - 福岳灵猪技能E

#include "Combat/Projectiles/DBAProjectile_Pig_E.h"
#include "Components/SphereComponent.h"

ADBAProjectile_Pig_E::ADBAProjectile_Pig_E()
{
	// 设置投射物属性
	Speed = 1200.0f;
	Radius = 30.0f;
	Damage = 50.0f;

	// 设置特效资源路径
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ProjectileVFXFinder(
		TEXT("/Game/VFX/Projectiles/Pig/P_Pig_E_Projectile.P_Pig_E_Projectile"));
	if (ProjectileVFXFinder.Succeeded())
	{
		ProjectileVFXAsset = ProjectileVFXFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ImpactVFXFinder(
		TEXT("/Game/VFX/Projectiles/Pig/P_Pig_E_Impact.P_Pig_E_Impact"));
	if (ImpactVFXFinder.Succeeded())
	{
		ImpactVFXAsset = ImpactVFXFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundCue> FlySFXFinder(
		TEXT("/Game/Audio/SFX/Projectiles/Pig/S_Pig_E_Fly.S_Pig_E_Fly"));
	if (FlySFXFinder.Succeeded())
	{
		FlySFXAsset = FlySFXFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundCue> ImpactSFXFinder(
		TEXT("/Game/Audio/SFX/Projectiles/Pig/S_Pig_E_Impact.S_Pig_E_Impact"));
	if (ImpactSFXFinder.Succeeded())
	{
		ImpactSFXAsset = ImpactSFXFinder.Object;
	}
}

void ADBAProjectile_Pig_E::BeginPlay()
{
	Super::BeginPlay();
}

void ADBAProjectile_Pig_E::InitializeProjectile(
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

	// 更新移动组件
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed * 1.5f;
	}

	// 更新碰撞半径
	if (USphereComponent* Sphere = Cast<USphereComponent>(RootComponent))
	{
		Sphere->SetSphereRadius(Radius);
	}

	// 加载飞行特效
	if (ProjectileVFXAsset.IsValid())
	{
		if (UParticleSystem* VFX = ProjectileVFXAsset.LoadSynchronous())
		{
			ProjectileVFX->SetTemplate(VFX);
		}
	}

	// 设置初始速度方向朝向目标
	if (InTarget)
	{
		FVector Direction = (InTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		ProjectileMovement->Velocity = Direction * Speed;
	}
}
