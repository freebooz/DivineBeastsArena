// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Preview/DBACharacterPreviewStage.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewActor.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewCameraRig.h"
#include "Engine/World.h"

ADBACharacterPreviewStage::ADBACharacterPreviewStage()
{
	bReplicates = false;
	SetCanBeDamaged(false);
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewStageRoot"));
	SetRootComponent(Root);
	SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(Root);
	CameraRigAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRigAnchor"));
	CameraRigAnchor->SetupAttachment(Root);
	BackgroundAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("BackgroundAnchor"));
	BackgroundAnchor->SetupAttachment(Root);
	VfxAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("VfxAnchor"));
	VfxAnchor->SetupAttachment(Root);
	KeyLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(Root);
	FillLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(Root);
	RimLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("RimLight"));
	RimLight->SetupAttachment(Root);
}

ADBACharacterPreviewStage* ADBACharacterPreviewStage::FindPlacedPreviewStage(UWorld* World)
{
	if (!IsValid(World) || World->GetNetMode() == NM_DedicatedServer)
	{
		return nullptr;
	}
	for (TActorIterator<ADBACharacterPreviewStage> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

ADBACharacterPreviewActor* ADBACharacterPreviewStage::EnsurePreviewActor()
{
	if (IsDedicatedServer())
	{
		return nullptr;
	}
	if (IsValid(PreviewActor))
	{
		return PreviewActor;
	}
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}
	TSubclassOf<ADBACharacterPreviewActor> ActorClass = PreviewActorClass;
	if (!ActorClass)
	{
		ActorClass = ADBACharacterPreviewActor::StaticClass();
	}
	PreviewActor = World->SpawnActor<ADBACharacterPreviewActor>(ActorClass, SpawnPoint->GetComponentTransform());
	if (!PreviewActor)
	{
		UE_LOG(LogDBAPreview, Error, TEXT("[角色预览] 无法创建 PreviewActor。"));
	}
	return PreviewActor;
}

ADBACharacterPreviewCameraRig* ADBACharacterPreviewStage::EnsureCameraRig()
{
	if (IsDedicatedServer())
	{
		return nullptr;
	}
	if (IsValid(CameraRig))
	{
		return CameraRig;
	}
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}
	TSubclassOf<ADBACharacterPreviewCameraRig> RigClass = CameraRigClass;
	if (!RigClass)
	{
		RigClass = ADBACharacterPreviewCameraRig::StaticClass();
	}
	CameraRig = World->SpawnActor<ADBACharacterPreviewCameraRig>(RigClass, CameraRigAnchor->GetComponentTransform());
	if (!CameraRig)
	{
		UE_LOG(LogDBAPreview, Error, TEXT("[角色预览] 无法创建 PreviewCameraRig。"));
	}
	return CameraRig;
}

void ADBACharacterPreviewStage::ReleasePreviewActor()
{
	if (IsValid(PreviewActor))
	{
		PreviewActor->ReleasePreviewResources();
		PreviewActor->Destroy();
	}
	PreviewActor = nullptr;
	if (IsValid(CameraRig))
	{
		CameraRig->Destroy();
	}
	CameraRig = nullptr;
}

bool ADBACharacterPreviewStage::IsDedicatedServer() const
{
	return GetNetMode() == NM_DedicatedServer;
}
