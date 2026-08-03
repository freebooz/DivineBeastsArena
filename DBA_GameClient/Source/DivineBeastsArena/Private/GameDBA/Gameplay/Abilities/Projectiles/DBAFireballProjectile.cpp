// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Gameplay/Abilities/Projectiles/DBAFireballProjectile.h"

#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"

ADBAFireballProjectile::ADBAFireballProjectile()
{
	SkillId = TEXT("Lobby.Fireball");
	Damage = 35.0f;
	Speed = 1450.0f;
	Radius = 42.0f;
	InitialLifeSpan = 4.0f;

	if (CollisionSphere)
	{
		CollisionSphere->InitSphereRadius(Radius);
	}
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed;
		ProjectileMovement->ProjectileGravityScale = 0.0f;
	}

	FireballCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FireballCore"));
	FireballCore->SetupAttachment(RootComponent);
	FireballCore->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FireballCore->SetRelativeScale3D(FVector(0.42f));
	FireballCore->SetHiddenInGame(true);
	FireballCore->SetVisibility(false);

	FireballLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireballLight"));
	FireballLight->SetupAttachment(RootComponent);
	FireballLight->SetLightColor(FireballColor);
	FireballLight->Intensity = 6500.0f;
	FireballLight->AttenuationRadius = 520.0f;
	FireballLight->bUseInverseSquaredFalloff = false;

	FireballLoopAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("FireballLoopAudio"));
	FireballLoopAudio->SetupAttachment(RootComponent);
	FireballLoopAudio->bAutoActivate = false;
	FireballLoopAudio->bAllowSpatialization = true;

	// 引擎内置 BasicShape 资源在引擎启动时已加载，使用 TSoftObjectPtr 软引用 Get() 直接获取已加载对象，
	// 避免 ConstructorHelpers 同步加载模式（符合 DBA.UI.EventAsync 异步策略与 DBA.DataAsset.NoHardcoding 策略）。
	TSoftObjectPtr<UStaticMesh> SphereMeshRef(FSoftObjectPath(TEXT("/Engine/BasicShapes/Sphere.Sphere")));
	if (UStaticMesh* SphereMesh = SphereMeshRef.Get())
	{
		FireballCore->SetStaticMesh(SphereMesh);
	}

	TSoftObjectPtr<UMaterialInterface> MaterialRef(FSoftObjectPath(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")));
	if (UMaterialInterface* Material = MaterialRef.Get())
	{
		FireballCoreMaterial = Material;
	}

	ProjectileNiagaraVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Projectile.NS_DBA_Fireball_Projectile")));
	ImpactNiagaraVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Impact.NS_DBA_Fireball_Impact")));
	FlySFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_MageFireball_Flight.SFX_MageFireball_Flight")));
	ImpactSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_MageFireball_Impact.SFX_MageFireball_Impact")));
}

void ADBAFireballProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (FireballCoreMaterial && FireballCore)
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(FireballCoreMaterial, this);
		if (DynamicMaterial)
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), FireballColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FireballColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), FireballColor * 8.0f);
			FireballCore->SetMaterial(0, DynamicMaterial);
		}
	}

	if (FireballCore)
	{
		FireballCore->SetHiddenInGame(true);
		FireballCore->SetVisibility(false);
		FireballCore->SetStaticMesh(nullptr);
	}
	if (ProjectileNiagaraVFX && !ProjectileNiagaraVFXAsset.IsNull())
	{
		if (UNiagaraSystem* BurningVFX = ProjectileNiagaraVFXAsset.Get())
		{
			ProjectileNiagaraVFX->SetAsset(BurningVFX);
			ProjectileNiagaraVFX->SetVisibility(true);
			ApplyNiagaraSkillParameters(ProjectileNiagaraVFX, ResolveNiagaraTargetLocation(), ResolveNiagaraDirection());
			ProjectileNiagaraVFX->Activate(true);
		}
		else
		{
			DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(this, ProjectileNiagaraVFXAsset, [this](UNiagaraSystem* BurningVFX)
			{
				if (!ProjectileNiagaraVFX || bProjectileHitProcessed)
				{
					return;
				}
				ProjectileNiagaraVFX->SetAsset(BurningVFX);
				ProjectileNiagaraVFX->SetVisibility(true);
				ApplyNiagaraSkillParameters(ProjectileNiagaraVFX, ResolveNiagaraTargetLocation(), ResolveNiagaraDirection());
				ProjectileNiagaraVFX->Activate(true);
			});
		}
	}
	// 飞行循环音效由通用投射物基类在 InitializeProjectile 中播放，确保运行时覆盖的技能音效也生效。
}

void ADBAFireballProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AgeSeconds += DeltaSeconds;
	const float Pulse = 1.0f + FMath::Sin(AgeSeconds * FireballPulseSpeed) * FireballPulseAmount;
	if (FireballCore)
	{
		FireballCore->SetRelativeScale3D(FVector(0.42f * Pulse));
		FireballCore->AddLocalRotation(FRotator(0.0f, 180.0f * DeltaSeconds, 90.0f * DeltaSeconds));
	}
	if (FireballLight)
	{
		FireballLight->Intensity = 6500.0f + 1600.0f * Pulse;
	}
}

void ADBAFireballProjectile::OnProjectileHit(AActor* HitActor, FVector HitLocation)
{
	if (FireballLight)
	{
		FireballLight->Deactivate();
	}
	if (FireballLoopAudio)
	{
		FireballLoopAudio->Stop();
	}
	Super::OnProjectileHit(HitActor, HitLocation);
}
