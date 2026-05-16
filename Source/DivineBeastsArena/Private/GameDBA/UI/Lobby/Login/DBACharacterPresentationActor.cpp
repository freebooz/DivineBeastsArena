// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"

#include "Animation/AnimationAsset.h"
#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Scene.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"

namespace
{
	constexpr float PreviewMeshDisplayScale = 1.0f;
	constexpr float PreviewMeshFloorZ = 2.0f;

	const TCHAR* const ZodiacPreviewMeshPaths[] = {
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Ox.SKM_DBA_Zodiac_Ox"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Tiger.SKM_DBA_Zodiac_Tiger"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rabbit.SKM_DBA_Zodiac_Rabbit"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Dragon.SKM_DBA_Zodiac_Dragon"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Snake.SKM_DBA_Zodiac_Snake"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Horse.SKM_DBA_Zodiac_Horse"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Goat.SKM_DBA_Zodiac_Goat"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Monkey.SKM_DBA_Zodiac_Monkey"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rooster.SKM_DBA_Zodiac_Rooster"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Dog.SKM_DBA_Zodiac_Dog"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Pig.SKM_DBA_Zodiac_Pig")
	};

	const TCHAR* const ZodiacPreviewMaterialPaths[] = {
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Rat.MI_DBA_Zodiac_Rat"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Ox.MI_DBA_Zodiac_Ox"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Tiger.MI_DBA_Zodiac_Tiger"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Rabbit.MI_DBA_Zodiac_Rabbit"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Dragon.MI_DBA_Zodiac_Dragon"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Snake.MI_DBA_Zodiac_Snake"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Horse.MI_DBA_Zodiac_Horse"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Goat.MI_DBA_Zodiac_Goat"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Monkey.MI_DBA_Zodiac_Monkey"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Rooster.MI_DBA_Zodiac_Rooster"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Dog.MI_DBA_Zodiac_Dog"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Pig.MI_DBA_Zodiac_Pig")
	};
}

ADBACharacterPresentationActor::ADBACharacterPresentationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	const FDBACharacterPresentationStageSpec Spec = GetReferenceStageSpec();

	StageRoot = CreateDefaultSubobject<USceneComponent>(TEXT("StageRoot"));
	RootComponent = StageRoot;

	PreviewMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMeshComponent"));
	PreviewMeshComponent->SetupAttachment(StageRoot);
	PreviewMeshComponent->SetRelativeLocation(FVector::ZeroVector);
	PreviewMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	PreviewMeshComponent->SetRelativeScale3D(FVector(PreviewMeshDisplayScale));
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMeshComponent->SetGenerateOverlapEvents(false);
	PreviewMeshComponent->SetVisibility(true);
	PreviewMeshComponent->SetHiddenInGame(false);
	PreviewMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	PresentationCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PresentationCamera"));
	PresentationCamera->SetupAttachment(StageRoot);
	PresentationCamera->bConstrainAspectRatio = false;
	PresentationCamera->SetAutoActivate(true);

	GroundPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundPlane"));
	GroundPlane->SetupAttachment(StageRoot);
	GroundPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GroundPlane->SetGenerateOverlapEvents(false);

	Pedestal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pedestal"));
	Pedestal->SetupAttachment(StageRoot);
	Pedestal->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Pedestal->SetGenerateOverlapEvents(false);

	BackdropPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackdropPlane"));
	BackdropPlane->SetupAttachment(StageRoot);
	BackdropPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BackdropPlane->SetGenerateOverlapEvents(false);

	LeftPillar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftPillar"));
	LeftPillar->SetupAttachment(StageRoot);
	LeftPillar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftPillar->SetGenerateOverlapEvents(false);

	RightPillar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightPillar"));
	RightPillar->SetupAttachment(StageRoot);
	RightPillar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightPillar->SetGenerateOverlapEvents(false);

	MoonDisc = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MoonDisc"));
	MoonDisc->SetupAttachment(StageRoot);
	MoonDisc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MoonDisc->SetGenerateOverlapEvents(false);

	KeyLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(StageRoot);
	KeyLight->SetMobility(EComponentMobility::Movable);
	KeyLight->SetCastShadows(true);
	KeyLight->SetLightColor(FLinearColor(1.0f, 0.86f, 0.62f));
	KeyLight->SetForwardShadingPriority(100);
	KeyLight->SetAffectTranslucentLighting(true);

	FillLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(StageRoot);
	FillLight->SetMobility(EComponentMobility::Movable);
	FillLight->SetCastShadows(false);
	FillLight->SetLightColor(FLinearColor(0.56f, 0.68f, 1.0f));
	FillLight->SetForwardShadingPriority(0);
	FillLight->SetAffectTranslucentLighting(false);

	RimLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("RimLight"));
	RimLight->SetupAttachment(StageRoot);
	RimLight->SetMobility(EComponentMobility::Movable);
	RimLight->SetCastShadows(false);
	RimLight->SetLightColor(FLinearColor(0.85f, 0.94f, 1.0f));
	RimLight->SetForwardShadingPriority(0);
	RimLight->SetAffectTranslucentLighting(false);

	FaceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FaceLight"));
	FaceLight->SetupAttachment(StageRoot);
	FaceLight->SetMobility(EComponentMobility::Movable);
	FaceLight->SetCastShadows(false);
	FaceLight->SetLightColor(FLinearColor(1.0f, 0.80f, 0.55f));

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(StageRoot);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->SetCastShadows(false);
	SkyLight->SetLightColor(FLinearColor(0.72f, 0.80f, 1.0f));

	AtmosphereFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("AtmosphereFog"));
	AtmosphereFog->SetupAttachment(StageRoot);
	AtmosphereFog->SetMobility(EComponentMobility::Movable);

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(StageRoot);
	PostProcess->SetMobility(EComponentMobility::Movable);
	PostProcess->bUnbound = false;
	PostProcess->BlendWeight = 1.0f;

	for (const TCHAR* MeshPath : ZodiacPreviewMeshPaths)
	{
		ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(MeshPath);
		if (MeshFinder.Succeeded() && MeshFinder.Object)
		{
			CookAnchorPreviewMeshes.Add(MeshFinder.Object);
		}
	}

	for (const TCHAR* MaterialPath : ZodiacPreviewMaterialPaths)
	{
		ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(MaterialPath);
		if (MaterialFinder.Succeeded() && MaterialFinder.Object)
		{
			CookAnchorPreviewMaterials.Add(MaterialFinder.Object);
		}
	}

	ApplyStageSpec();
}

FDBACharacterPresentationStageSpec ADBACharacterPresentationActor::GetReferenceStageSpec()
{
	return FDBACharacterPresentationStageSpec();
}

void ADBACharacterPresentationActor::BeginPlay()
{
	Super::BeginPlay();

	ConfigureStageVisuals();
	ApplyStageSpec();
	ApplyPreviewAssets(EDBAZodiac::Rat);
}

void ADBACharacterPresentationActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ConfigureStageVisuals();
	ApplyStageSpec();
}

void ADBACharacterPresentationActor::SetPreviewZodiac(EDBAZodiac Zodiac)
{
	ApplyPreviewAssets(Zodiac == EDBAZodiac::None ? EDBAZodiac::Rat : Zodiac);
}

void ADBACharacterPresentationActor::AddPreviewYaw(float DeltaYawDegrees)
{
	if (PreviewMeshComponent)
	{
		PreviewMeshComponent->AddLocalRotation(FRotator(0.0f, DeltaYawDegrees, 0.0f));
	}
}

void ADBACharacterPresentationActor::ActivatePresentationCamera(APlayerController* PlayerController, float BlendTime)
{
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}

	if (PlayerController)
	{
		if (PresentationCamera)
		{
			PresentationCamera->Activate(true);
		}
		PlayerController->SetViewTargetWithBlend(this, BlendTime);
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] Activated presentation camera. Actor=%s Camera=%s Location=%s"),
			*GetName(),
			PresentationCamera && PresentationCamera->IsActive() ? TEXT("Active") : TEXT("Inactive"),
			*GetActorLocation().ToString());
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPresentationActor] Failed to activate presentation camera: no PlayerController."));
	}
}

void ADBACharacterPresentationActor::ConfigureStageVisuals()
{
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (PlaneMesh)
	{
		if (GroundPlane)
		{
			GroundPlane->SetStaticMesh(PlaneMesh);
		}
		if (BackdropPlane)
		{
			BackdropPlane->SetStaticMesh(PlaneMesh);
		}
		if (MoonDisc)
		{
			MoonDisc->SetStaticMesh(PlaneMesh);
		}
	}

	if (CylinderMesh && Pedestal)
	{
		Pedestal->SetStaticMesh(CylinderMesh);
	}

	if (CubeMesh)
	{
		if (LeftPillar)
		{
			LeftPillar->SetStaticMesh(CubeMesh);
		}
		if (RightPillar)
		{
			RightPillar->SetStaticMesh(CubeMesh);
		}
	}

	if (!MoonDisc && SphereMesh)
	{
		MoonDisc->SetStaticMesh(SphereMesh);
	}
}

void ADBACharacterPresentationActor::ApplyStageSpec()
{
	const FDBACharacterPresentationStageSpec Spec = GetReferenceStageSpec();

	if (PresentationCamera)
	{
		PresentationCamera->SetRelativeLocation(Spec.CameraLocation);
		PresentationCamera->SetRelativeRotation(Spec.CameraRotation);
		PresentationCamera->SetFieldOfView(Spec.CameraFOV);
	}

	if (GroundPlane)
	{
		GroundPlane->SetRelativeLocation(FVector(-42.0f, 0.0f, -5.0f));
		GroundPlane->SetRelativeRotation(FRotator::ZeroRotator);
		GroundPlane->SetRelativeScale3D(Spec.GroundScale);
	}

	if (Pedestal)
	{
		Pedestal->SetRelativeLocation(FVector(-8.0f, 0.0f, -8.0f));
		Pedestal->SetRelativeRotation(FRotator::ZeroRotator);
		Pedestal->SetRelativeScale3D(FVector(1.65f, 1.65f, 0.18f));
		Pedestal->SetVisibility(Spec.bUsePedestal);
		Pedestal->SetHiddenInGame(!Spec.bUsePedestal);
	}

	if (BackdropPlane)
	{
		BackdropPlane->SetRelativeLocation(FVector(-235.0f, 0.0f, 132.0f));
		BackdropPlane->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
		BackdropPlane->SetRelativeScale3D(FVector(6.4f, 4.8f, 1.0f));
	}

	if (LeftPillar)
	{
		LeftPillar->SetRelativeLocation(FVector(-120.0f, -150.0f, 78.0f));
		LeftPillar->SetRelativeRotation(FRotator(0.0f, -8.0f, 0.0f));
		LeftPillar->SetRelativeScale3D(FVector(0.20f, 0.20f, 1.95f));
	}

	if (RightPillar)
	{
		RightPillar->SetRelativeLocation(FVector(-120.0f, 150.0f, 78.0f));
		RightPillar->SetRelativeRotation(FRotator(0.0f, 8.0f, 0.0f));
		RightPillar->SetRelativeScale3D(FVector(0.20f, 0.20f, 1.95f));
	}

	if (MoonDisc)
	{
		MoonDisc->SetRelativeLocation(FVector(-238.0f, 0.0f, 188.0f));
		MoonDisc->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
		MoonDisc->SetRelativeScale3D(FVector(1.35f, 1.35f, 1.0f));
	}

	if (KeyLight)
	{
		KeyLight->SetRelativeRotation(FRotator(-42.0f, -38.0f, 0.0f));
		KeyLight->SetIntensity(Spec.KeyLightIntensity);
	}

	if (FillLight)
	{
		FillLight->SetRelativeRotation(FRotator(-14.0f, 146.0f, 0.0f));
		FillLight->SetIntensity(Spec.FillLightIntensity);
	}

	if (RimLight)
	{
		RimLight->SetRelativeRotation(FRotator(-18.0f, 218.0f, 0.0f));
		RimLight->SetIntensity(Spec.RimLightIntensity);
	}

	if (FaceLight)
	{
		FaceLight->SetRelativeLocation(FVector(205.0f, 0.0f, 126.0f));
		FaceLight->SetIntensity(Spec.FaceLightIntensity);
		FaceLight->SetAttenuationRadius(560.0f);
	}

	if (SkyLight)
	{
		SkyLight->SetIntensity(Spec.SkyLightIntensity);
	}

	if (PostProcess)
	{
		PostProcess->SetRelativeLocation(FVector(120.0f, 0.0f, 96.0f));
		FPostProcessSettings& PPS = PostProcess->Settings;
		PPS.bOverride_AutoExposureMethod = true;
		PPS.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
		PPS.bOverride_AutoExposureBias = true;
		PPS.AutoExposureBias = -1.15f;
		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = FVector4(0.92f, 0.92f, 0.92f, 1.0f);
	}

	if (AtmosphereFog)
	{
		AtmosphereFog->SetVisibility(Spec.bUseAtmosphericFog);
		AtmosphereFog->SetHiddenInGame(!Spec.bUseAtmosphericFog);
		AtmosphereFog->SetFogDensity(0.028f);
		AtmosphereFog->SetFogHeightFalloff(0.18f);
		AtmosphereFog->SetFogInscatteringColor(FLinearColor(0.20f, 0.42f, 0.36f, 1.0f));
		AtmosphereFog->SetStartDistance(72.0f);
	}
}

void ADBACharacterPresentationActor::ApplyPreviewAssets(EDBAZodiac Zodiac)
{
	if (!PreviewMeshComponent)
	{
		return;
	}

	const TArray<FString> MeshCandidates = {
		GetMeshPathByZodiac(Zodiac),
		GetLegacyMeshPathByZodiac(Zodiac),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat")
	};

	USkeletalMesh* ResolvedMesh = nullptr;
	FString ResolvedMeshPath;
	for (const FString& MeshPath : MeshCandidates)
	{
		if (!MeshPath.IsEmpty())
		{
			if (USkeletalMesh* CandidateMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath))
			{
				ResolvedMesh = CandidateMesh;
				ResolvedMeshPath = MeshPath;
				UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] Loaded mesh: %s Skeleton=%s"),
					*MeshPath,
					CandidateMesh->GetSkeleton() ? TEXT("Valid") : TEXT("None"));
				break;
			}
		}
	}

	if (!ResolvedMesh)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[CharacterPresentationActor] Failed to load any preview skeletal mesh."));
		return;
	}

	PreviewMeshComponent->SetSkeletalMesh(ResolvedMesh);
	PreviewMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	PreviewMeshComponent->SetRelativeScale3D(FVector(PreviewMeshDisplayScale));
	const FBox MeshBox = ResolvedMesh->GetBounds().GetBox();
	const float MeshBottomOffsetZ = MeshBox.IsValid
		? (-MeshBox.Min.Z * PreviewMeshDisplayScale) + PreviewMeshFloorZ
		: 0.0f;
	PreviewMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, MeshBottomOffsetZ));
	PreviewMeshComponent->SetBoundsScale(2.0f);
	PreviewMeshComponent->SetVisibility(true);
	PreviewMeshComponent->SetHiddenInGame(false);
	PreviewMeshComponent->UpdateBounds();
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] Mesh applied. Scale=%.2f OffsetZ=%.2f Bounds=%s"),
		PreviewMeshDisplayScale,
		MeshBottomOffsetZ,
		*PreviewMeshComponent->Bounds.GetBox().ToString());

	if (ResolvedMesh->GetSkeleton())
	{
		const FString IdleAnimationPath = GetIdleAnimationPathByZodiac(Zodiac);
		if (!IdleAnimationPath.IsEmpty())
		{
			if (UAnimationAsset* IdleAnimation = LoadObject<UAnimationAsset>(nullptr, *IdleAnimationPath))
			{
				PreviewMeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
				PreviewMeshComponent->SetAnimation(IdleAnimation);
				PreviewMeshComponent->Play(true);
			}
		}
	}
	else
	{
		PreviewMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}

	const bool bUsingZodiacMesh = ResolvedMeshPath.Contains(TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/"))
		|| ResolvedMeshPath.Contains(TEXT("/Game/Models/Zodiac/"));
	const FString MaterialPath = bUsingZodiacMesh
		? GetMaterialPathByZodiac(Zodiac)
		: TEXT("/Game/DBA/Characters/Mannequins/Materials/Instances/Manny/MI_Manny_01.MI_Manny_01");
	if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPath))
	{
		const int32 MaterialSlotCount = PreviewMeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
		{
			PreviewMeshComponent->SetMaterial(MaterialIndex, Material);
		}
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] Applied material: %s"), *MaterialPath);
	}
}

FString ADBACharacterPresentationActor::GetMeshPathByZodiac(EDBAZodiac Zodiac)
{
	return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat");
}

FString ADBACharacterPresentationActor::GetLegacyMeshPathByZodiac(EDBAZodiac Zodiac)
{
	return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat");
}

FString ADBACharacterPresentationActor::GetIdleAnimationPathByZodiac(EDBAZodiac Zodiac)
{
	return TEXT("");
}

FString ADBACharacterPresentationActor::GetMaterialPathByZodiac(EDBAZodiac Zodiac)
{
	switch (Zodiac)
	{
	case EDBAZodiac::Rat: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Rat.MI_DBA_Zodiac_Rat");
	case EDBAZodiac::Ox: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Ox.MI_DBA_Zodiac_Ox");
	case EDBAZodiac::Tiger: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Tiger.MI_DBA_Zodiac_Tiger");
	case EDBAZodiac::Rabbit: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Rabbit.MI_DBA_Zodiac_Rabbit");
	case EDBAZodiac::Dragon: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Dragon.MI_DBA_Zodiac_Dragon");
	case EDBAZodiac::Snake: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Snake.MI_DBA_Zodiac_Snake");
	case EDBAZodiac::Horse: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Horse.MI_DBA_Zodiac_Horse");
	case EDBAZodiac::Goat: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Goat.MI_DBA_Zodiac_Goat");
	case EDBAZodiac::Monkey: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Monkey.MI_DBA_Zodiac_Monkey");
	case EDBAZodiac::Rooster: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Rooster.MI_DBA_Zodiac_Rooster");
	case EDBAZodiac::Dog: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Dog.MI_DBA_Zodiac_Dog");
	case EDBAZodiac::Pig: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Pig.MI_DBA_Zodiac_Pig");
	default: return TEXT("/Game/DBA/Characters/Mannequins/Materials/Instances/Manny/MI_Manny_01.MI_Manny_01");
	}
}
