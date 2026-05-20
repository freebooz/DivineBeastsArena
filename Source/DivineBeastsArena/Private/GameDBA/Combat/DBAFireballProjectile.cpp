// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/DBAFireballProjectile.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

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

	FireballLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireballLight"));
	FireballLight->SetupAttachment(RootComponent);
	FireballLight->SetLightColor(FireballColor);
	FireballLight->Intensity = 6500.0f;
	FireballLight->AttenuationRadius = 520.0f;
	FireballLight->bUseInverseSquaredFalloff = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshFinder.Succeeded())
	{
		FireballCore->SetStaticMesh(SphereMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (MaterialFinder.Succeeded())
	{
		FireballCoreMaterial = MaterialFinder.Object;
	}

	ProjectileNiagaraVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Projectile.NS_DBA_Fireball_Projectile")));
	ImpactNiagaraVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Impact.NS_DBA_Fireball_Impact")));
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
	Super::OnProjectileHit(HitActor, HitLocation);
}
