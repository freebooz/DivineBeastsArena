// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/DBACharacterPreviewActor.h"

#include "Animation/AnimationAsset.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"
#include "Engine/SkeletalMesh.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Net/UnrealNetwork.h"

namespace
{
	constexpr float PreviewMeshDisplayScale = 1.0f;
	constexpr float PreviewMeshFloorZ = 2.0f;
}

ADBACharacterPreviewActor::ADBACharacterPreviewActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bAlwaysRelevant = true;
	SetReplicates(true);
	SetReplicateMovement(true);

	PreviewRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewRoot"));
	RootComponent = PreviewRoot;

	PreviewMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMeshComponent"));
	PreviewMeshComponent->SetupAttachment(PreviewRoot);
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMeshComponent->SetGenerateOverlapEvents(false);
	PreviewMeshComponent->SetVisibility(true);
	PreviewMeshComponent->SetComponentTickEnabled(true);
	PreviewMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	PreviewMeshComponent->SetRelativeRotation(ADBACharacterPresentationActor::GetPreviewMeshPlayerFacingRotation());

	ZodiacTintLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ZodiacTintLight"));
	ZodiacTintLight->SetupAttachment(PreviewRoot);
	ZodiacTintLight->SetMobility(EComponentMobility::Movable);
	ZodiacTintLight->SetCastShadows(false);
	ZodiacTintLight->SetIntensity(650000.0f);
	ZodiacTintLight->SetAttenuationRadius(760.0f);
	ZodiacTintLight->SetRelativeLocation(FVector(170.0f, 0.0f, 110.0f));

	ApplyPreviewAssets(CurrentZodiac);
}

void ADBACharacterPreviewActor::BeginPlay()
{
	Super::BeginPlay();
	ApplyPreviewAssets(CurrentZodiac);
}

void ADBACharacterPreviewActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (FMath::Abs(RotationSpeedDegreesPerSecond) > KINDA_SMALL_NUMBER)
	{
		AddActorLocalRotation(FRotator(0.0f, RotationSpeedDegreesPerSecond * DeltaSeconds, 0.0f));
	}
}

void ADBACharacterPreviewActor::SetPreviewZodiac(EDBAZodiac Zodiac)
{
	CurrentZodiac = Zodiac == EDBAZodiac::None ? EDBAZodiac::Rat : Zodiac;
	ApplyPreviewAssets(CurrentZodiac);
	if (HasAuthority())
	{
		ForceNetUpdate();
	}
}

void ADBACharacterPreviewActor::SetRotationSpeed(float InDegreesPerSecond)
{
	RotationSpeedDegreesPerSecond = InDegreesPerSecond;
}

void ADBACharacterPreviewActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADBACharacterPreviewActor, CurrentZodiac);
}

void ADBACharacterPreviewActor::OnRep_CurrentZodiac()
{
	ApplyPreviewAssets(CurrentZodiac);
}

void ADBACharacterPreviewActor::ApplyPreviewAssets(EDBAZodiac Zodiac)
{
	if (!PreviewMeshComponent)
	{
		return;
	}

	const TArray<FString> MeshCandidates = {
		ADBACharacterPresentationActor::GetPreviewMeshPathForZodiac(Zodiac),
		ADBACharacterPresentationActor::GetPreviewLegacyMeshPathForZodiac(Zodiac),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat")
	};

	USkeletalMesh* ResolvedMesh = nullptr;
	for (const FString& MeshPath : MeshCandidates)
	{
		if (!MeshPath.IsEmpty())
		{
			ResolvedMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
			if (ResolvedMesh)
			{
				UE_LOG(LogDBAUI, Log, TEXT("[CharacterPreviewActor] Loaded mesh: %s"), *MeshPath);
				break;
			}
		}
	}

	if (!ResolvedMesh)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[CharacterPreviewActor] Failed to load any preview skeletal mesh."));
		return;
	}
	PreviewMeshComponent->SetSkeletalMesh(ResolvedMesh);
	PreviewMeshComponent->SetRelativeRotation(ADBACharacterPresentationActor::GetPreviewMeshPlayerFacingRotation());
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
	if (ZodiacTintLight)
	{
		ZodiacTintLight->SetLightColor(ADBACharacterPresentationActor::GetPreviewTintForZodiac(Zodiac));
		ZodiacTintLight->SetVisibility(true);
		ZodiacTintLight->SetHiddenInGame(false);
	}

	if (ResolvedMesh->GetSkeleton())
	{
		const FString IdleAnimationPath = ADBACharacterPresentationActor::GetPreviewIdleAnimationPathForZodiac(Zodiac);
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
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPreviewActor] Mesh has no skeleton, skip idle animation."));
	}

	ADBACharacterPresentationActor::ApplyZodiacMaterialToMesh(PreviewMeshComponent, Zodiac, this);
}
