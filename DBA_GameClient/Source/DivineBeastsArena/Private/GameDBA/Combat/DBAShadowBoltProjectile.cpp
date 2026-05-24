// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: original warlock-like shadow projectile with dark core, purple wake, and corrupt impact.
*/

#include "GameDBA/Combat/DBAShadowBoltProjectile.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

ADBAShadowBoltProjectile::ADBAShadowBoltProjectile()
{
	SkillId = TEXT("Lobby.ShadowBolt");
	Damage = 44.0f;
	Speed = 1580.0f;
	Radius = 40.0f;
	DamageElement = EDBAElement::Gold;
	InitialLifeSpan = 4.5f;

	if (CollisionSphere)
	{
		CollisionSphere->InitSphereRadius(Radius);
	}
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed * 1.45f;
		ProjectileMovement->ProjectileGravityScale = 0.0f;
	}

	ShadowCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShadowCore"));
	ShadowCore->SetupAttachment(RootComponent);
	ShadowCore->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShadowCore->SetCastShadow(false);

	ShadowHalo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShadowHalo"));
	ShadowHalo->SetupAttachment(RootComponent);
	ShadowHalo->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShadowHalo->SetCastShadow(false);

	ShadowWake = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ShadowWake"));
	ShadowWake->SetupAttachment(RootComponent);
	ShadowWake->bAutoActivate = false;

	ShadowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ShadowLight"));
	ShadowLight->SetupAttachment(RootComponent);
	ShadowLight->SetLightColor(ShadowColor);
	ShadowLight->Intensity = 4600.0f;
	ShadowLight->AttenuationRadius = 430.0f;
	ShadowLight->bUseInverseSquaredFalloff = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshFinder.Succeeded())
	{
		ShadowCore->SetStaticMesh(SphereMeshFinder.Object);
		ShadowHalo->SetStaticMesh(SphereMeshFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (MaterialFinder.Succeeded())
	{
		ShadowMaterial = MaterialFinder.Object;
	}

	ShadowCore->SetRelativeScale3D(FVector(0.28f, 0.28f, 0.42f));
	ShadowHalo->SetRelativeScale3D(FVector(0.42f, 0.42f, 0.28f));
	ProjectileNiagaraVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_PoisonSkullFish.NS_PoisonSkullFish")));
	WakeVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Magic.NS_Hit_Magic")));
	ImpactNiagaraVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Poison.NS_Hit_Poison")));
	SecondaryImpactVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Blood_Normal.NS_Hit_Blood_Normal")));
	FlySFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_ShadowBolt_Flight.SFX_ShadowBolt_Flight")));
	ImpactSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_ShadowBolt_Impact.SFX_ShadowBolt_Impact")));
}

void ADBAShadowBoltProjectile::BeginPlay()
{
	Super::BeginPlay();

	ApplyShadowMaterial(ShadowCore, 9.0f, 1.0f);
	ApplyShadowMaterial(ShadowHalo, 5.5f, 0.55f);
	ActivateWake();
}

void ADBAShadowBoltProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AgeSeconds += DeltaSeconds;
	const float Pulse = 1.0f + FMath::Sin(AgeSeconds * 10.0f) * 0.18f;
	if (ShadowCore)
	{
		ShadowCore->SetRelativeScale3D(FVector(0.28f * Pulse, 0.28f * Pulse, 0.42f * (1.0f + Pulse * 0.08f)));
		ShadowCore->AddLocalRotation(FRotator(260.0f * DeltaSeconds, 120.0f * DeltaSeconds, 420.0f * DeltaSeconds));
	}
	if (ShadowHalo)
	{
		ShadowHalo->SetRelativeScale3D(FVector(0.42f + 0.08f * Pulse, 0.42f + 0.08f * Pulse, 0.28f));
		ShadowHalo->AddLocalRotation(FRotator(-160.0f * DeltaSeconds, 320.0f * DeltaSeconds, 110.0f * DeltaSeconds));
	}
	if (ShadowLight)
	{
		ShadowLight->Intensity = 3800.0f + 1200.0f * Pulse;
	}
}

void ADBAShadowBoltProjectile::OnProjectileHit(AActor* HitActor, FVector HitLocation)
{
	if (ShadowLight)
	{
		ShadowLight->Deactivate();
	}
	if (ShadowWake)
	{
		ShadowWake->Deactivate();
	}
	SpawnImpactLayer(SecondaryImpactVFXAsset, HitLocation, FRotator::ZeroRotator, FVector(0.86f));
	Super::OnProjectileHit(HitActor, HitLocation);
}

void ADBAShadowBoltProjectile::ApplyShadowMaterial(UStaticMeshComponent* Mesh, float EmissiveStrength, float Alpha) const
{
	if (!Mesh || !ShadowMaterial)
	{
		return;
	}

	if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(ShadowMaterial, Mesh))
	{
		const FLinearColor LayerColor(ShadowColor.R, ShadowColor.G, ShadowColor.B, Alpha);
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), LayerColor);
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), LayerColor);
		DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), LayerColor * EmissiveStrength);
		Mesh->SetMaterial(0, DynamicMaterial);
	}
}

void ADBAShadowBoltProjectile::ActivateWake() const
{
	if (!ShadowWake || WakeVFXAsset.IsNull() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (UNiagaraSystem* VFX = WakeVFXAsset.LoadSynchronous())
	{
		ShadowWake->SetAsset(VFX);
		ShadowWake->SetVisibility(true);
		ShadowWake->Activate(true);
	}
}

void ADBAShadowBoltProjectile::SpawnImpactLayer(const TSoftObjectPtr<UNiagaraSystem>& Asset, const FVector& Location, const FRotator& Rotation, const FVector& Scale) const
{
	if (Asset.IsNull() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (UNiagaraSystem* Impact = Asset.LoadSynchronous())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			Impact,
			Location,
			Rotation,
			Scale,
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true);
	}
}
