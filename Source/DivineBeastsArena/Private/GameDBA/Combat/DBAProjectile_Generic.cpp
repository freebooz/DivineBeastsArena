// Copyright Freebooz Games, Inc. All Rights Reserved.
// 泛化投射物类实现

#include "GameDBA/Combat/DBAProjectile_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Components/SphereComponent.h"

ADBAProjectile_Generic::ADBAProjectile_Generic()
{
	// 默认值
	Speed = 1000.0f;
	Radius = 50.0f;
	Damage = 50.0f;
}

void ADBAProjectile_Generic::InitializeProjectile(
	FName InSkillId,
	AActor* InOwner,
	AActor* InTarget,
	float InDamage,
	float InSpeed,
	float InRadius)
{
	// 如果有DataTable，先尝试从DataTable加载
	if (ProjectileTable && !InSkillId.IsNone())
	{
		LoadFromDataTable(InSkillId);
	}

	// 然后用传入的参数覆盖
	SkillId = InSkillId;
	ProjectileOwner = InOwner;
	TargetActor = InTarget;

	// 只有当传入值大于0时才覆盖DataTable的值
	if (InDamage > 0)
	{
		Damage = InDamage;
	}
	if (InSpeed > 0)
	{
		Speed = InSpeed;
	}
	if (InRadius > 0)
	{
		Radius = InRadius;
	}

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

	// 设置初始速度方向指向目标
	if (InTarget)
	{
		FVector Direction = (InTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		ProjectileMovement->Velocity = Direction * Speed;
	}

	UE_LOG(LogDBACombat, Log, TEXT("[DBAProjectile_Generic] 初始化投射物: %s, Speed=%.1f, Radius=%.1f, Damage=%.1f"),
		*InSkillId.ToString(), Speed, Radius, Damage);
}

void ADBAProjectile_Generic::LoadFromDataTable(FName InSkillId)
{
	if (!ProjectileTable)
	{
		return;
	}

	static const FString ContextString = TEXT("DBAProjectile_Generic");
	FDBAProjectileDataRow* Row = ProjectileTable->FindRow<FDBAProjectileDataRow>(InSkillId, ContextString, false);

	if (!Row)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAProjectile_Generic] 未找到投射物配置: %s"), *InSkillId.ToString());
		return;
	}

	// 应用配置
	Speed = Row->Speed;
	Radius = Row->Radius;
	Damage = Row->Damage;
	ProjectileVFXAsset = Row->ProjectileVFX;
	ImpactVFXAsset = Row->ImpactVFX;
	FlySFXAsset = Row->FlySFX;
	ImpactSFXAsset = Row->ImpactSFX;

	UE_LOG(LogDBACombat, Log, TEXT("[DBAProjectile_Generic] 从DataTable加载: %s, Speed=%.1f"), *InSkillId.ToString(), Speed);
}
