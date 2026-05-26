// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: original high-fidelity frost projectile visual using existing project Niagara/SFX assets.
- This is not a frame-accurate clone of any third-party spell effect.
*/

#include "GameDBA/Combat/DBAFrostShardProjectile.h"

#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameDBA/Utilities/DBAAsyncAssetLoader.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	void ConfigureFrostMesh(
		UStaticMeshComponent* Mesh,
		UStaticMesh* StaticMesh,
		const FVector& RelativeLocation,
		const FRotator& RelativeRotation,
		const FVector& RelativeScale)
	{
		if (!Mesh)
		{
			return;
		}

		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetRelativeLocation(RelativeLocation);
		Mesh->SetRelativeRotation(RelativeRotation);
		Mesh->SetRelativeScale3D(RelativeScale);
		Mesh->SetCastShadow(false);
		if (StaticMesh)
		{
			Mesh->SetStaticMesh(StaticMesh);
		}
	}
}

ADBAFrostShardProjectile::ADBAFrostShardProjectile()
{
	SkillId = TEXT("Common.FrostShard");
	Damage = 32.0f;
	Speed = 1840.0f;
	Radius = 38.0f;
	DamageElement = EDBAElement::Water;
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

	FrostCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrostCore"));
	FrostCore->SetupAttachment(RootComponent);

	FrostShardA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrostShardA"));
	FrostShardA->SetupAttachment(RootComponent);

	FrostShardB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrostShardB"));
	FrostShardB->SetupAttachment(RootComponent);

	FrostShardC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrostShardC"));
	FrostShardC->SetupAttachment(RootComponent);

	FrostShardD = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrostShardD"));
	FrostShardD->SetupAttachment(RootComponent);

	FrostCrystalWake = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FrostCrystalWake"));
	FrostCrystalWake->SetupAttachment(RootComponent);
	FrostCrystalWake->bAutoActivate = false;

	FrostMistWake = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FrostMistWake"));
	FrostMistWake->SetupAttachment(RootComponent);
	FrostMistWake->bAutoActivate = false;

	FrostSpiralWake = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FrostSpiralWake"));
	FrostSpiralWake->SetupAttachment(RootComponent);
	FrostSpiralWake->bAutoActivate = false;

	FrostLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FrostLight"));
	FrostLight->SetupAttachment(RootComponent);
	FrostLight->SetLightColor(FrostColor);
	FrostLight->Intensity = 5600.0f;
	FrostLight->AttenuationRadius = 520.0f;
	FrostLight->bUseInverseSquaredFalloff = false;

	FrostTipLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FrostTipLight"));
	FrostTipLight->SetupAttachment(RootComponent);
	FrostTipLight->SetRelativeLocation(FVector(42.0f, 0.0f, 0.0f));
	FrostTipLight->SetLightColor(FLinearColor(0.74f, 0.96f, 1.0f, 1.0f));
	FrostTipLight->Intensity = 2300.0f;
	FrostTipLight->AttenuationRadius = 260.0f;
	FrostTipLight->bUseInverseSquaredFalloff = false;

	UStaticMesh* ConeMesh = nullptr;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMeshFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeMeshFinder.Succeeded())
	{
		ConeMesh = ConeMeshFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (MaterialFinder.Succeeded())
	{
		FrostCoreMaterial = MaterialFinder.Object;
	}

	ConfigureFrostMesh(FrostCore, ConeMesh, FVector(18.0f, 0.0f, 0.0f), FRotator(0.0f, 90.0f, 0.0f), FVector(0.18f, 0.18f, 0.78f));
	ConfigureFrostMesh(FrostShardA, ConeMesh, FVector(-24.0f, 16.0f, 9.0f), FRotator(24.0f, 112.0f, 18.0f), FVector(0.07f, 0.07f, 0.36f));
	ConfigureFrostMesh(FrostShardB, ConeMesh, FVector(-31.0f, -14.0f, -7.0f), FRotator(-18.0f, 75.0f, -24.0f), FVector(0.06f, 0.06f, 0.32f));
	ConfigureFrostMesh(FrostShardC, ConeMesh, FVector(-42.0f, 6.0f, -15.0f), FRotator(34.0f, 130.0f, 64.0f), FVector(0.05f, 0.05f, 0.24f));
	ConfigureFrostMesh(FrostShardD, ConeMesh, FVector(-48.0f, -7.0f, 14.0f), FRotator(-36.0f, 58.0f, -68.0f), FVector(0.05f, 0.05f, 0.26f));

	ProjectileNiagaraVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_IceDart.NS_IceDart")));
	CrystalWakeVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_IceCrystal.NS_IceCrystal")));
	MistWakeVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Iceicle3D.NS_Iceicle3D")));
	SpiralWakeVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_MagicLanceShuriken.NS_MagicLanceShuriken")));
	ImpactNiagaraVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Ice_01.NS_Hit_Ice_01")));
	SecondaryImpactVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_ColdBlood.NS_Hit_ColdBlood")));
	RingImpactVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_IceCrystal.NS_IceCrystal")));
	FlySFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_FrostShard_Flight.SFX_FrostShard_Flight")));
	ImpactSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_FrostShard_Impact.SFX_FrostShard_Impact")));
}

void ADBAFrostShardProjectile::BeginPlay()
{
	Super::BeginPlay();

	ApplyFrostMaterial(FrostCore, 8.5f, 1.0f);
	ApplyFrostMaterial(FrostShardA, 5.0f, 0.92f);
	ApplyFrostMaterial(FrostShardB, 5.0f, 0.88f);
	ApplyFrostMaterial(FrostShardC, 4.0f, 0.78f);
	ApplyFrostMaterial(FrostShardD, 4.0f, 0.78f);

	if (ProjectileNiagaraVFX && !ProjectileNiagaraVFXAsset.IsNull())
	{
		if (UNiagaraSystem* MainVFX = ProjectileNiagaraVFXAsset.Get())
		{
			ProjectileNiagaraVFX->SetAsset(MainVFX);
			ProjectileNiagaraVFX->SetVisibility(true);
			ProjectileNiagaraVFX->Activate(true);
		}
		else
		{
			DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(this, ProjectileNiagaraVFXAsset, [this](UNiagaraSystem* MainVFX)
			{
				if (!ProjectileNiagaraVFX || bProjectileHitProcessed)
				{
					return;
				}
				ProjectileNiagaraVFX->SetAsset(MainVFX);
				ProjectileNiagaraVFX->SetVisibility(true);
				ProjectileNiagaraVFX->Activate(true);
			});
		}
	}

	ActivateNiagaraComponent(FrostCrystalWake, CrystalWakeVFXAsset);
	ActivateNiagaraComponent(FrostMistWake, MistWakeVFXAsset);
	ActivateNiagaraComponent(FrostSpiralWake, SpiralWakeVFXAsset);
}

void ADBAFrostShardProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AgeSeconds += DeltaSeconds;
	const float Pulse = 1.0f + FMath::Sin(AgeSeconds * FrostPulseSpeed) * FrostPulseAmount;
	const float Orbit = FMath::DegreesToRadians(AgeSeconds * 560.0f);
	if (FrostCore)
	{
		FrostCore->SetRelativeScale3D(FVector(0.18f * Pulse, 0.18f * Pulse, 0.78f + 0.04f * Pulse));
		FrostCore->AddLocalRotation(FRotator(520.0f * DeltaSeconds, 0.0f, 220.0f * DeltaSeconds));
	}
	if (FrostShardA)
	{
		FrostShardA->SetRelativeLocation(FVector(-24.0f, 16.0f + FMath::Sin(AgeSeconds * 9.0f) * 3.5f, 9.0f + FMath::Cos(AgeSeconds * 8.0f) * 2.5f));
		FrostShardA->AddLocalRotation(FRotator(260.0f * DeltaSeconds, 110.0f * DeltaSeconds, 380.0f * DeltaSeconds));
	}
	if (FrostShardB)
	{
		FrostShardB->SetRelativeLocation(FVector(-31.0f, -14.0f + FMath::Cos(AgeSeconds * 8.5f) * 3.0f, -7.0f + FMath::Sin(AgeSeconds * 9.5f) * 2.0f));
		FrostShardB->AddLocalRotation(FRotator(-220.0f * DeltaSeconds, 160.0f * DeltaSeconds, 340.0f * DeltaSeconds));
	}
	if (FrostShardC)
	{
		FrostShardC->SetRelativeLocation(FVector(-42.0f, FMath::Sin(Orbit) * 12.0f, FMath::Cos(Orbit) * 12.0f));
		FrostShardC->AddLocalRotation(FRotator(360.0f * DeltaSeconds, -190.0f * DeltaSeconds, 240.0f * DeltaSeconds));
	}
	if (FrostShardD)
	{
		FrostShardD->SetRelativeLocation(FVector(-49.0f, FMath::Sin(Orbit + PI) * 10.0f, FMath::Cos(Orbit + PI) * 10.0f));
		FrostShardD->AddLocalRotation(FRotator(-340.0f * DeltaSeconds, 180.0f * DeltaSeconds, -280.0f * DeltaSeconds));
	}
	if (FrostLight)
	{
		FrostLight->Intensity = 4700.0f + 1700.0f * Pulse;
	}
	if (FrostTipLight)
	{
		FrostTipLight->Intensity = 1900.0f + 900.0f * Pulse;
	}
}

void ADBAFrostShardProjectile::OnProjectileHit(AActor* HitActor, FVector HitLocation)
{
	DeactivateLayeredVFX();
	SpawnImpactLayer(SecondaryImpactVFXAsset, HitLocation + GetActorForwardVector() * 8.0f, GetActorRotation(), FVector(0.78f));
	SpawnImpactLayer(RingImpactVFXAsset, HitLocation, FRotator(0.0f, GetActorRotation().Yaw, 0.0f), FVector(0.95f));

	Super::OnProjectileHit(HitActor, HitLocation);
}

void ADBAFrostShardProjectile::PreloadPresentationAssets()
{
	Super::PreloadPresentationAssets();

	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(CrystalWakeVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(MistWakeVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(SpiralWakeVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(SecondaryImpactVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(RingImpactVFXAsset, Paths);
	DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
}

void ADBAFrostShardProjectile::ApplyFrostMaterial(UStaticMeshComponent* Mesh, float EmissiveStrength, float Alpha) const
{
	if (!Mesh || !FrostCoreMaterial)
	{
		return;
	}

	if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(FrostCoreMaterial, Mesh))
	{
		const FLinearColor LayerColor(FrostColor.R, FrostColor.G, FrostColor.B, Alpha);
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), LayerColor);
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), LayerColor);
		DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), LayerColor * EmissiveStrength);
		Mesh->SetMaterial(0, DynamicMaterial);
	}
}

void ADBAFrostShardProjectile::ActivateNiagaraComponent(UNiagaraComponent* Component, const TSoftObjectPtr<UNiagaraSystem>& Asset) const
{
	if (!Component || Asset.IsNull() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (UNiagaraSystem* VFX = Asset.Get())
	{
		if (!Component || bProjectileHitProcessed)
		{
			return;
		}
		Component->SetAsset(VFX);
		Component->SetVisibility(true);
		Component->Activate(true);
	}
	else
	{
		// Optional frost wake layers are warmed by UDBAPlayableSkillComponent.
		// Do not compile or load them from the cast path; the base projectile core remains visible.
	}
}

void ADBAFrostShardProjectile::DeactivateLayeredVFX()
{
	if (FrostLight)
	{
		FrostLight->Deactivate();
	}
	if (FrostTipLight)
	{
		FrostTipLight->Deactivate();
	}
	if (FrostCrystalWake)
	{
		FrostCrystalWake->Deactivate();
	}
	if (FrostMistWake)
	{
		FrostMistWake->Deactivate();
	}
	if (FrostSpiralWake)
	{
		FrostSpiralWake->Deactivate();
	}
}

void ADBAFrostShardProjectile::SpawnImpactLayer(
	const TSoftObjectPtr<UNiagaraSystem>& Asset,
	const FVector& Location,
	const FRotator& Rotation,
	const FVector& Scale) const
{
	if (Asset.IsNull() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (UNiagaraSystem* Impact = Asset.Get())
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
	else
	{
		// Optional impact layers are skipped if not warmed yet to keep key-press casting hitch-free.
	}
}
