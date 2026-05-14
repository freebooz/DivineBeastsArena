// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"

#include "Animation/AnimationAsset.h"
#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"

ADBACharacterPresentationActor::ADBACharacterPresentationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	StageRoot = CreateDefaultSubobject<USceneComponent>(TEXT("StageRoot"));
	RootComponent = StageRoot;

	PreviewMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMeshComponent"));
	PreviewMeshComponent->SetupAttachment(StageRoot);
	PreviewMeshComponent->SetRelativeLocation(FVector::ZeroVector);
	PreviewMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	PreviewMeshComponent->SetRelativeScale3D(FVector(1.08f));
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMeshComponent->SetGenerateOverlapEvents(false);
	PreviewMeshComponent->SetVisibility(true);
	PreviewMeshComponent->SetHiddenInGame(false);
	PreviewMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	PresentationCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PresentationCamera"));
	PresentationCamera->SetupAttachment(StageRoot);
	PresentationCamera->SetRelativeLocation(FVector(310.0f, 0.0f, 98.0f));
	PresentationCamera->SetRelativeRotation(FRotator(-5.0f, 180.0f, 0.0f));
	PresentationCamera->SetFieldOfView(42.0f);
	PresentationCamera->bConstrainAspectRatio = false;
	PresentationCamera->SetAutoActivate(true);

	GroundPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundPlane"));
	GroundPlane->SetupAttachment(StageRoot);
	GroundPlane->SetRelativeLocation(FVector(-18.0f, 0.0f, -2.0f));
	GroundPlane->SetRelativeScale3D(FVector(4.2f, 4.2f, 1.0f));
	GroundPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GroundPlane->SetGenerateOverlapEvents(false);

	BackdropPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackdropPlane"));
	BackdropPlane->SetupAttachment(StageRoot);
	BackdropPlane->SetRelativeLocation(FVector(-170.0f, 0.0f, 120.0f));
	BackdropPlane->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	BackdropPlane->SetRelativeScale3D(FVector(3.8f, 3.8f, 1.0f));
	BackdropPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BackdropPlane->SetGenerateOverlapEvents(false);

	KeyLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(StageRoot);
	KeyLight->SetRelativeRotation(FRotator(-36.0f, -30.0f, 0.0f));
	KeyLight->SetMobility(EComponentMobility::Movable);
	KeyLight->SetCastShadows(true);
	KeyLight->SetIntensity(65000.0f);
	KeyLight->SetLightColor(FLinearColor(1.0f, 0.86f, 0.62f));

	FillLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(StageRoot);
	FillLight->SetRelativeRotation(FRotator(-10.0f, 145.0f, 0.0f));
	FillLight->SetMobility(EComponentMobility::Movable);
	FillLight->SetCastShadows(false);
	FillLight->SetIntensity(21000.0f);
	FillLight->SetLightColor(FLinearColor(0.56f, 0.68f, 1.0f));

	RimLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("RimLight"));
	RimLight->SetupAttachment(StageRoot);
	RimLight->SetRelativeRotation(FRotator(-12.0f, 215.0f, 0.0f));
	RimLight->SetMobility(EComponentMobility::Movable);
	RimLight->SetCastShadows(false);
	RimLight->SetIntensity(36000.0f);
	RimLight->SetLightColor(FLinearColor(0.85f, 0.94f, 1.0f));

	FaceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FaceLight"));
	FaceLight->SetupAttachment(StageRoot);
	FaceLight->SetRelativeLocation(FVector(185.0f, 0.0f, 118.0f));
	FaceLight->SetMobility(EComponentMobility::Movable);
	FaceLight->SetCastShadows(false);
	FaceLight->SetIntensity(12000.0f);
	FaceLight->SetAttenuationRadius(520.0f);
	FaceLight->SetLightColor(FLinearColor(1.0f, 0.80f, 0.55f));

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(StageRoot);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->SetCastShadows(false);
	SkyLight->SetIntensity(3.2f);
	SkyLight->SetLightColor(FLinearColor(0.72f, 0.80f, 1.0f));
}

void ADBACharacterPresentationActor::BeginPlay()
{
	Super::BeginPlay();

	ConfigureStageVisuals();
	ApplyPreviewAssets(EDBAZodiac::Rat);
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
		TEXT("/Game/DBA/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"),
		TEXT("/Game/DBA/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat")
	};

	USkeletalMesh* ResolvedMesh = nullptr;
	for (const FString& MeshPath : MeshCandidates)
	{
		if (!MeshPath.IsEmpty())
		{
			if (USkeletalMesh* CandidateMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath))
			{
				if (CandidateMesh->GetSkeleton())
				{
					ResolvedMesh = CandidateMesh;
					UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] Loaded mesh: %s"), *MeshPath);
					break;
				}

				UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPresentationActor] Mesh has no skeleton, skipping for world stage: %s"), *MeshPath);
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
	PreviewMeshComponent->SetVisibility(true);
	PreviewMeshComponent->SetHiddenInGame(false);
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] Mesh applied. Bounds=%s"), *PreviewMeshComponent->Bounds.GetBox().ToString());

	if (UAnimationAsset* IdleAnimation = LoadObject<UAnimationAsset>(nullptr, *GetIdleAnimationPathByZodiac(Zodiac)))
	{
		PreviewMeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		PreviewMeshComponent->SetAnimation(IdleAnimation);
		PreviewMeshComponent->Play(true);
	}

	if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *GetMaterialPathByZodiac(Zodiac)))
	{
		PreviewMeshComponent->SetMaterial(0, Material);
	}
}

FString ADBACharacterPresentationActor::GetMeshPathByZodiac(EDBAZodiac Zodiac)
{
	switch (Zodiac)
	{
	case EDBAZodiac::Rat: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat");
	case EDBAZodiac::Ox: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Ox.SKM_DBA_Zodiac_Ox");
	case EDBAZodiac::Tiger: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Tiger.SKM_DBA_Zodiac_Tiger");
	case EDBAZodiac::Rabbit: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rabbit.SKM_DBA_Zodiac_Rabbit");
	case EDBAZodiac::Dragon: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Dragon.SKM_DBA_Zodiac_Dragon");
	case EDBAZodiac::Snake: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Snake.SKM_DBA_Zodiac_Snake");
	case EDBAZodiac::Horse: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Horse.SKM_DBA_Zodiac_Horse");
	case EDBAZodiac::Goat: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Goat.SKM_DBA_Zodiac_Goat");
	case EDBAZodiac::Monkey: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Monkey.SKM_DBA_Zodiac_Monkey");
	case EDBAZodiac::Rooster: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rooster.SKM_DBA_Zodiac_Rooster");
	case EDBAZodiac::Dog: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Dog.SKM_DBA_Zodiac_Dog");
	case EDBAZodiac::Pig: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Pig.SKM_DBA_Zodiac_Pig");
	default: return TEXT("/Game/DBA/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny");
	}
}

FString ADBACharacterPresentationActor::GetIdleAnimationPathByZodiac(EDBAZodiac Zodiac)
{
	return TEXT("/Game/DBA/Characters/Mannequins/Animations/Manny/MM_Idle.MM_Idle");
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
