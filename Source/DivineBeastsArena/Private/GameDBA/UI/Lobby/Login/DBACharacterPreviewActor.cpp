// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/DBACharacterPreviewActor.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Net/UnrealNetwork.h"

namespace
{
	constexpr float PreviewActorMeshDisplayScale = 1.0f;
	constexpr float PreviewActorMeshFloorZ = 2.0f;

	void EnsureRosalesMeshUsesRosalesSkeleton(USkeletalMesh* Mesh, const FString& MeshPath)
	{
		if (!Mesh || !MeshPath.Contains(TEXT("/Game/DBA/Characters/Rosales/")))
		{
			return;
		}

		if (USkeleton* RosalesSkeleton = LoadObject<USkeleton>(nullptr, TEXT("/Game/DBA/Characters/Rosales/Meshes/SKEL_Rosales.SKEL_Rosales")))
		{
			Mesh->SetSkeleton(RosalesSkeleton);
		}
	}

	bool IsLobbyGameplayWorldForPreviewActor(const UWorld* World)
	{
		if (!World || !World->PersistentLevel)
		{
			return false;
		}

		const FString LevelPath = World->PersistentLevel->GetOutermost()->GetName();
		return LevelPath.Contains(TEXT("LobbyMap")) || LevelPath.Contains(TEXT("MainLobby"));
	}

}

ADBACharacterPreviewActor::ADBACharacterPreviewActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bAlwaysRelevant = true;
	bReplicates = true;
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
}

void ADBACharacterPreviewActor::BeginPlay()
{
	Super::BeginPlay();

	if (IsLobbyGameplayWorldForPreviewActor(GetWorld()))
	{
		Destroy();
		return;
	}

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

	const TArray<FString> MeshCandidates = ADBACharacterPresentationActor::GetLobbyDisplayMeshCandidatePathsForZodiac(Zodiac);

	USkeletalMesh* ResolvedMesh = nullptr;
	FString ResolvedMeshPath;
	for (const FString& MeshPath : MeshCandidates)
	{
		if (!MeshPath.IsEmpty())
		{
			if (USkeletalMesh* CandidateMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath))
			{
				EnsureRosalesMeshUsesRosalesSkeleton(CandidateMesh, MeshPath);
				ResolvedMesh = CandidateMesh;
				ResolvedMeshPath = MeshPath;
				break;
			}
		}
	}

	if (!ResolvedMesh)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[CharacterPreviewActor] Failed to load any preview skeletal mesh."));
		return;
	}
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterPreviewActor] Loaded mesh: %s Skeleton=%s"),
		*ResolvedMeshPath,
		ResolvedMesh->GetSkeleton() ? TEXT("Valid") : TEXT("None"));
	PreviewMeshComponent->SetSkeletalMesh(ResolvedMesh);
	PreviewMeshComponent->SetRelativeRotation(ADBACharacterPresentationActor::GetPreviewMeshPlayerFacingRotation());
	PreviewMeshComponent->SetRelativeScale3D(FVector(PreviewActorMeshDisplayScale));
	const FBox MeshBox = ResolvedMesh->GetBounds().GetBox();
	const float MeshBottomOffsetZ = MeshBox.IsValid
		? (-MeshBox.Min.Z * PreviewActorMeshDisplayScale) + PreviewActorMeshFloorZ
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

	if (!ADBACharacterPresentationActor::ApplyLobbyDisplayAnimationToMesh(PreviewMeshComponent, ResolvedMeshPath, Zodiac))
	{
		PreviewMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPreviewActor] Mesh has no skeleton, skip idle animation."));
	}

	ADBACharacterPresentationActor::ApplyZodiacMaterialToMesh(PreviewMeshComponent, Zodiac, this);
}
