// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Preview/DBACharacterPreviewStage.h"

#include "Components/AudioComponent.h"
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
	FiveCampThemeAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("FiveCampThemeAudio"));
	FiveCampThemeAudio->SetupAttachment(Root);
	FiveCampThemeAudio->bAutoActivate = false;
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
	ClearFiveCampTheme();
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

void ADBACharacterPreviewStage::ApplyFiveCampTheme(const FDBAFiveCampPreviewTheme& InTheme)
{
	// Dedicated Server 从不创建或加载前台主题资源；调用方仍做一次防御性检查，避免未来接入绕过 Subsystem。
	if (IsDedicatedServer() || InTheme.FiveCamp == EDBAFiveCamp::None)
	{
		return;
	}

	CurrentFiveCampTheme = InTheme;
	if (FiveCampThemeAudio)
	{
		FiveCampThemeAudio->Stop();
		FiveCampThemeAudio->SetSound(InTheme.ThemeSound);
		if (InTheme.ThemeSound)
		{
			FiveCampThemeAudio->Play();
		}
	}

	// 舞台蓝图只消费已经异步解析完成的资源，负责把它们挂到 Background/Vfx 锚点的具体美术组件。
	// 这里不访问 PreviewActor，不会改写生肖外观，更不会把五营映射成对局 TeamId。
	BP_OnFiveCampThemeApplied(CurrentFiveCampTheme);
}

void ADBACharacterPreviewStage::ClearFiveCampTheme()
{
	if (FiveCampThemeAudio)
	{
		FiveCampThemeAudio->Stop();
		FiveCampThemeAudio->SetSound(nullptr);
	}
	CurrentFiveCampTheme = FDBAFiveCampPreviewTheme();
	if (!IsDedicatedServer())
	{
		BP_OnFiveCampThemeCleared();
	}
}

bool ADBACharacterPreviewStage::IsDedicatedServer() const
{
	return GetNetMode() == NM_DedicatedServer;
}
