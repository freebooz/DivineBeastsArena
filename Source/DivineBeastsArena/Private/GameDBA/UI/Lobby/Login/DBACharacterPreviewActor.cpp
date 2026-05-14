// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/DBACharacterPreviewActor.h"

#include "Animation/AnimBlueprint.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkeletalMesh.h"

ADBACharacterPreviewActor::ADBACharacterPreviewActor()
{
	PrimaryActorTick.bCanEverTick = true;

	PreviewMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMeshComponent"));
	RootComponent = PreviewMeshComponent;
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMeshComponent->SetGenerateOverlapEvents(false);
	PreviewMeshComponent->SetVisibility(true);
	PreviewMeshComponent->SetComponentTickEnabled(true);
	PreviewMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void ADBACharacterPreviewActor::BeginPlay()
{
	Super::BeginPlay();
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
	ApplyPreviewAssets(Zodiac);
}

void ADBACharacterPreviewActor::SetRotationSpeed(float InDegreesPerSecond)
{
	RotationSpeedDegreesPerSecond = InDegreesPerSecond;
}

void ADBACharacterPreviewActor::ApplyPreviewAssets(EDBAZodiac Zodiac)
{
	if (!PreviewMeshComponent)
	{
		return;
	}

	const FString MeshPath = GetMeshPathByZodiac(Zodiac);
	if (!MeshPath.IsEmpty())
	{
		if (USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath))
		{
			PreviewMeshComponent->SetSkeletalMesh(Mesh);
		}
	}

	const FString AnimBlueprintPath = GetAnimBlueprintPathByZodiac(Zodiac);
	if (!AnimBlueprintPath.IsEmpty())
	{
		if (UAnimBlueprint* AnimBlueprint = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath))
		{
			PreviewMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			PreviewMeshComponent->SetAnimInstanceClass(AnimBlueprint->GeneratedClass);
		}
	}

	const FString MaterialPath = GetMaterialPathByZodiac(Zodiac);
	if (!MaterialPath.IsEmpty())
	{
		if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPath))
		{
			PreviewMeshComponent->SetMaterial(0, Material);
		}
	}
}

FString ADBACharacterPreviewActor::GetMeshPathByZodiac(EDBAZodiac Zodiac)
{
	return TEXT("/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin");
}

FString ADBACharacterPreviewActor::GetAnimBlueprintPathByZodiac(EDBAZodiac Zodiac)
{
	return TEXT("/Game/DBA/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny");
}

FString ADBACharacterPreviewActor::GetMaterialPathByZodiac(EDBAZodiac Zodiac)
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
	default:
		return TEXT("/Game/DBA/Characters/Mannequins/Materials/Instances/Manny/MI_Manny_01.MI_Manny_01");
	}
}
