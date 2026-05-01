// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - 夜隐灵鼠技能Q

#include "Combat/Projectiles/DBAProjectile_Rat_Q.h"
#include "Components/SphereComponent.h"

ADBAProjectile_Rat_Q::ADBAProjectile_Rat_Q()
{
	// 设置投射物属性
	Speed = 1200.0f;
	Radius = 30.0f;
	Damage = 50.0f;

	// 设置特效资源路径
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ProjectileVFXFinder(
		TEXT("/Game/VFX/Projectiles/Rat/P_Rat_Q_Projectile.P_Rat_Q_Projectile"));
	if (ProjectileVFXFinder.Succeeded())
	{
		ProjectileVFXAsset = ProjectileVFXFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ImpactVFXFinder(
		TEXT("/Game/VFX/Projectiles/Rat/P_Rat_Q_Impact.P_Rat_Q_Impact"));
	if (ImpactVFXFinder.Succeeded())
	{
		ImpactVFXAsset = ImpactVFXFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundCue> FlySFXFinder(
		TEXT("/Game/Audio/SFX/Projectiles/Rat/S_Rat_Q_Fly.S_Rat_Q_Fly"));
	if (FlySFXFinder.Succeeded())
	{
		FlySFXAsset = FlySFXFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundCue> ImpactSFXFinder(
		TEXT("/Game/Audio/SFX/Projectiles/Rat/S_Rat_Q_Impact.S_Rat_Q_Impact"));
	if (ImpactSFXFinder.Succeeded())
	{
		ImpactSFXAsset = ImpactSFXFinder.Object;
	}
}

void ADBAProjectile_Rat_Q::BeginPlay()
{
	Super::BeginPlay();
}

void ADBAProjectile_Rat_Q::InitializeProjectile(
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
