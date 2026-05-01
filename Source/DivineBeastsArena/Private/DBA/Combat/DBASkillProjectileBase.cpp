// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物基类

#include "DBA/Combat/DBASkillProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADBASkillProjectileBase::ADBASkillProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件
	USphereComponent* SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->InitSphereRadius(Radius);
	SphereComp->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = SphereComp;

	// 创建静态网格组件 (可见性)
	UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 创建投射物移动组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed * 1.5f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bInitialVelocityInLocalSpace = false;

	// 创建VFX组件
	ProjectileVFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ProjectileVFX"));
	ProjectileVFX->SetupAttachment(RootComponent);
	ProjectileVFX->bAutoActivate = true;

	// 初始参数
	bReplicates = true;
	SetReplicateMovement(true);
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
	Damage = InDamage;
	Speed = InSpeed;
	Radius = InRadius;

	// 更新移动组件速度
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

	if (USphereComponent* Sphere = Cast<USphereComponent>(RootComponent))
	{
		Sphere->SetSphereRadius(Radius);
	}
}

void ADBASkillProjectileBase::OnProjectileHit(AActor* HitActor, FVector HitLocation)
{
	// 播放命中特效
	if (ImpactVFXAsset.IsValid())
	{
		if (UParticleSystem* VFX = ImpactVFXAsset.LoadSynchronous())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, HitLocation, FRotator::ZeroRotator, true);
		}
	}

	// 播放命中音效
	if (ImpactSFXAsset.IsValid())
	{
		if (USoundBase* SFX = ImpactSFXAsset.LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, HitLocation);
		}
	}

	// 应用伤害
	if (HitActor && Damage > 0)
	{
		FPointDamageEvent DamageEvent;
		DamageEvent.Damage = Damage;
		DamageEvent.HitInfo = FHitResult(HitActor, nullptr, HitLocation, FVector::ZeroVector);
		HitActor->TakeDamage(Damage, DamageEvent, ProjectileOwner ? ProjectileOwner->GetInstigatorController() : nullptr, ProjectileOwner);
	}

	// 调用蓝图事件
	BP_OnProjectileHit(HitActor, HitLocation);

	// 销毁投射物
	Destroy();
}

void ADBASkillProjectileBase::SetCollisionChannel(ECollisionChannel Channel)
{
	if (USphereComponent* Sphere = Cast<USphereComponent>(RootComponent))
	{
		Sphere->SetCollisionObjectType(Channel);
	}
}