// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"

#include "Animation/AnimationAsset.h"
#include "Animation/Skeleton.h"
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
#include "Engine/Texture.h"
#include "EngineUtils.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimInstance.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	constexpr float PreviewMeshDisplayScale = 1.0f;
	constexpr float PreviewMeshFloorZ = 2.0f;

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

	UMaterialInterface* LoadReliableFallbackMaterial()
	{
		if (UMaterialInterface* RuntimeTintMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/DBA/Materials/M_DBA_RuntimeTint.M_DBA_RuntimeTint")))
		{
			return RuntimeTintMaterial;
		}
		return LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}

	bool IsDefaultErrorMaterial(const UMaterialInterface* Material)
	{
		if (!Material)
		{
			return true;
		}

		const UMaterial* ResolvedMaterial = Material->GetMaterial();
		if (!ResolvedMaterial)
		{
			return true;
		}

		const FString MaterialPath = ResolvedMaterial->GetPathName();
		return MaterialPath.Contains(TEXT("/Engine/EngineMaterials/DefaultMaterial"))
			|| MaterialPath.Contains(TEXT("/Engine/EngineMaterials/WorldGridMaterial"));
	}

	bool IsRosalesMeshPath(const FString& MeshPath)
	{
		return MeshPath.Contains(TEXT("/Game/DBA/Characters/Rosales/"));
	}

	void ApplyStageSurfaceMaterial(UStaticMeshComponent* MeshComponent, UObject* Outer, const FLinearColor& BaseColor, float Roughness, float Specular, float NormalStrength, float EmissiveStrength = 0.0f)
	{
		if (!MeshComponent || !Outer)
		{
			return;
		}

		UMaterialInterface* BaseMaterial = LoadReliableFallbackMaterial();
		if (!BaseMaterial)
		{
			return;
		}

		if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Outer))
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Tint"), BaseColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), BaseColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), BaseColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("PrimaryColor"), BaseColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), BaseColor);
			DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), Roughness);
			DynamicMaterial->SetScalarParameterValue(TEXT("Specular"), Specular);
			DynamicMaterial->SetScalarParameterValue(TEXT("NormalStrength"), NormalStrength);
			DynamicMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), EmissiveStrength);
			MeshComponent->SetMaterial(0, DynamicMaterial);
		}
	}
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
	PreviewMeshComponent->SetRelativeRotation(GetPreviewMeshPlayerFacingRotation());
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
	KeyLight->SetUseTemperature(true);
	KeyLight->SetTemperature(10000.0f);
	KeyLight->SetLightSourceAngle(2.0f);
	KeyLight->SetForwardShadingPriority(100);
	KeyLight->SetAffectTranslucentLighting(true);
	KeyLight->ContactShadowLength = 0.2f;
	KeyLight->ContactShadowCastingIntensity = 1.0f;
	KeyLight->SetVolumetricScatteringIntensity(1.0f);

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
	FaceLight->SetLightColor(FLinearColor(0.25f, 0.45f, 1.0f));
	FaceLight->SetVolumetricScatteringIntensity(0.3f);

	LeftPillarRedLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LeftPillarRedLight"));
	LeftPillarRedLight->SetupAttachment(StageRoot);
	LeftPillarRedLight->SetMobility(EComponentMobility::Movable);
	LeftPillarRedLight->SetCastShadows(true);
	LeftPillarRedLight->SetLightColor(FLinearColor(1.0f, 0.23f, 0.08f));
	LeftPillarRedLight->SetVolumetricScatteringIntensity(2.5f);

	RightPillarRedLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RightPillarRedLight"));
	RightPillarRedLight->SetupAttachment(StageRoot);
	RightPillarRedLight->SetMobility(EComponentMobility::Movable);
	RightPillarRedLight->SetCastShadows(true);
	RightPillarRedLight->SetLightColor(FLinearColor(1.0f, 0.23f, 0.08f));
	RightPillarRedLight->SetVolumetricScatteringIntensity(2.5f);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(StageRoot);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->SetCastShadows(false);
	SkyLight->SetLightColor(FLinearColor(0.35f, 0.45f, 0.65f));
	SkyLight->SetLowerHemisphereColor(FLinearColor(0.005f, 0.008f, 0.015f));
	SkyLight->bLowerHemisphereIsBlack = true;
	SkyLight->SetRealTimeCapture(true);

	AtmosphereFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("AtmosphereFog"));
	AtmosphereFog->SetupAttachment(StageRoot);
	AtmosphereFog->SetMobility(EComponentMobility::Movable);

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(StageRoot);
	PostProcess->SetMobility(EComponentMobility::Movable);
	PostProcess->bUnbound = true;
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

ADBACharacterPresentationActor* ADBACharacterPresentationActor::ResolveSharedPresentationStage(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ADBACharacterPresentationActor> It(World); It; ++It)
	{
		if (IsValid(*It))
		{
			return *It;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return World->SpawnActor<ADBACharacterPresentationActor>(
		ADBACharacterPresentationActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams);
}

void ADBACharacterPresentationActor::ReleaseSharedPresentationStage(ADBACharacterPresentationActor*& InOutActor)
{
	InOutActor = nullptr;
}

FString ADBACharacterPresentationActor::GetPreviewMeshPathForZodiac(EDBAZodiac Zodiac)
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
	default: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat");
	}
}

FString ADBACharacterPresentationActor::GetPreviewLegacyMeshPathForZodiac(EDBAZodiac Zodiac)
{
	switch (Zodiac)
	{
	case EDBAZodiac::Rat: return TEXT("/Game/Models/Zodiac/Rat/SK_Rat_Mesh.SK_Rat_Mesh");
	case EDBAZodiac::Ox: return TEXT("/Game/Models/Zodiac/Ox/SK_Ox_Mesh.SK_Ox_Mesh");
	case EDBAZodiac::Tiger: return TEXT("/Game/Models/Zodiac/Tiger/SK_Tiger_Mesh.SK_Tiger_Mesh");
	case EDBAZodiac::Rabbit: return TEXT("/Game/Models/Zodiac/Rabbit/SK_Rabbit_Mesh.SK_Rabbit_Mesh");
	case EDBAZodiac::Dragon: return TEXT("/Game/Models/Zodiac/Dragon/SK_Dragon_Mesh.SK_Dragon_Mesh");
	case EDBAZodiac::Snake: return TEXT("/Game/Models/Zodiac/Snake/SK_Snake_Mesh.SK_Snake_Mesh");
	case EDBAZodiac::Horse: return TEXT("/Game/Models/Zodiac/Horse/SK_Horse_Mesh.SK_Horse_Mesh");
	case EDBAZodiac::Goat: return TEXT("/Game/Models/Zodiac/Goat/SK_Goat_Mesh.SK_Goat_Mesh");
	case EDBAZodiac::Monkey: return TEXT("/Game/Models/Zodiac/Monkey/SK_Monkey_Mesh.SK_Monkey_Mesh");
	case EDBAZodiac::Rooster: return TEXT("/Game/Models/Zodiac/Rooster/SK_Rooster_Mesh.SK_Rooster_Mesh");
	case EDBAZodiac::Dog: return TEXT("/Game/Models/Zodiac/Dog/SK_Dog_Mesh.SK_Dog_Mesh");
	case EDBAZodiac::Pig: return TEXT("/Game/Models/Zodiac/Pig/SK_Pig_Mesh.SK_Pig_Mesh");
	default: return TEXT("/Game/Models/Zodiac/Rat/SK_Rat_Mesh.SK_Rat_Mesh");
	}
}

TArray<FString> ADBACharacterPresentationActor::GetLobbyDisplayMeshCandidatePathsForZodiac(EDBAZodiac Zodiac)
{
	return {
		TEXT("/Game/DBA/Characters/Rosales/Meshes/SK_Rosales.SK_Rosales"),
		TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP.TutorialTPP"),
		GetPreviewMeshPathForZodiac(Zodiac),
		GetPreviewLegacyMeshPathForZodiac(Zodiac),
		TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat")
	};
}

FString ADBACharacterPresentationActor::GetLobbyDisplayAnimBlueprintPathForMeshPath(const FString& MeshPath, EDBAZodiac Zodiac)
{
	if (MeshPath.Contains(TEXT("/Game/DBA/Characters/Rosales/")))
	{
		return TEXT("/Game/DBA/Characters/Rosales/AnimationBP/ABP_Rosales.ABP_Rosales_C");
	}

	if (MeshPath.Contains(TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP")))
	{
		return TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP_AnimBlueprint.TutorialTPP_AnimBlueprint_C");
	}

	switch (Zodiac)
	{
	case EDBAZodiac::Rat: return TEXT("/Game/Animation/Zodiac/Rat/ABP_Rat.ABP_Rat_C");
	case EDBAZodiac::Ox: return TEXT("/Game/Animation/Zodiac/Ox/ABP_Ox.ABP_Ox_C");
	case EDBAZodiac::Tiger: return TEXT("/Game/Animation/Zodiac/Tiger/ABP_Tiger.ABP_Tiger_C");
	case EDBAZodiac::Rabbit: return TEXT("/Game/Animation/Zodiac/Rabbit/ABP_Rabbit.ABP_Rabbit_C");
	case EDBAZodiac::Dragon: return TEXT("/Game/Animation/Zodiac/Dragon/ABP_Dragon.ABP_Dragon_C");
	case EDBAZodiac::Snake: return TEXT("/Game/Animation/Zodiac/Snake/ABP_Snake.ABP_Snake_C");
	case EDBAZodiac::Horse: return TEXT("/Game/Animation/Zodiac/Horse/ABP_Horse.ABP_Horse_C");
	case EDBAZodiac::Goat: return TEXT("/Game/Animation/Zodiac/Goat/ABP_Goat.ABP_Goat_C");
	case EDBAZodiac::Monkey: return TEXT("/Game/Animation/Zodiac/Monkey/ABP_Monkey.ABP_Monkey_C");
	case EDBAZodiac::Rooster: return TEXT("/Game/Animation/Zodiac/Rooster/ABP_Rooster.ABP_Rooster_C");
	case EDBAZodiac::Dog: return TEXT("/Game/Animation/Zodiac/Dog/ABP_Dog.ABP_Dog_C");
	case EDBAZodiac::Pig: return TEXT("/Game/Animation/Zodiac/Pig/ABP_Pig.ABP_Pig_C");
	default: return TEXT("/Game/Animation/Zodiac/Rat/ABP_Rat.ABP_Rat_C");
	}
}

FString ADBACharacterPresentationActor::GetPreviewIdleAnimationPathForZodiac(EDBAZodiac Zodiac)
{
	return TEXT("");
}

FString ADBACharacterPresentationActor::GetPreviewMaterialPathForZodiac(EDBAZodiac Zodiac)
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
	default: return TEXT("/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Rat.MI_DBA_Zodiac_Rat");
	}
}

FLinearColor ADBACharacterPresentationActor::GetPreviewTintForZodiac(EDBAZodiac Zodiac)
{
	switch (Zodiac)
	{
	case EDBAZodiac::Rat: return FLinearColor(0.18f, 0.48f, 1.00f, 1.0f);
	case EDBAZodiac::Ox: return FLinearColor(0.96f, 0.52f, 0.12f, 1.0f);
	case EDBAZodiac::Tiger: return FLinearColor(1.00f, 0.26f, 0.04f, 1.0f);
	case EDBAZodiac::Rabbit: return FLinearColor(0.22f, 0.94f, 0.52f, 1.0f);
	case EDBAZodiac::Dragon: return FLinearColor(1.00f, 0.78f, 0.02f, 1.0f);
	case EDBAZodiac::Snake: return FLinearColor(0.04f, 0.78f, 0.32f, 1.0f);
	case EDBAZodiac::Horse: return FLinearColor(1.00f, 0.08f, 0.04f, 1.0f);
	case EDBAZodiac::Goat: return FLinearColor(0.92f, 0.74f, 0.38f, 1.0f);
	case EDBAZodiac::Monkey: return FLinearColor(0.08f, 0.82f, 1.00f, 1.0f);
	case EDBAZodiac::Rooster: return FLinearColor(1.00f, 0.18f, 0.06f, 1.0f);
	case EDBAZodiac::Dog: return FLinearColor(0.10f, 0.34f, 1.00f, 1.0f);
	case EDBAZodiac::Pig: return FLinearColor(1.00f, 0.34f, 0.54f, 1.0f);
	default: return FLinearColor(0.18f, 0.48f, 1.00f, 1.0f);
	}
}

FRotator ADBACharacterPresentationActor::GetPreviewMeshPlayerFacingRotation()
{
	return FRotator(0.0f, -90.0f, 0.0f);
}

bool ADBACharacterPresentationActor::ApplyLobbyDisplayAnimationToMesh(USkeletalMeshComponent* MeshComponent, const FString& MeshPath, EDBAZodiac Zodiac)
{
	if (!MeshComponent || !MeshComponent->GetSkeletalMeshAsset() || !MeshComponent->GetSkeletalMeshAsset()->GetSkeleton())
	{
		return false;
	}

	if (IsRosalesMeshPath(MeshPath))
	{
		const FString RosalesIdleAnimationPath(TEXT("/Game/DBA/Characters/Rosales/Animations/AN_Standing_Idle.AN_Standing_Idle"));
		if (UAnimationAsset* IdleAnimation = LoadObject<UAnimationAsset>(nullptr, *RosalesIdleAnimationPath))
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			MeshComponent->SetAnimation(IdleAnimation);
			MeshComponent->Play(true);
			UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] 已应用 Rosales 大厅待机动画：%s"), *RosalesIdleAnimationPath);
			return true;
		}
	}

	const FString AnimBlueprintPath = GetLobbyDisplayAnimBlueprintPathForMeshPath(MeshPath, Zodiac);
	if (!AnimBlueprintPath.IsEmpty())
	{
		if (UClass* AnimClass = LoadClass<UAnimInstance>(nullptr, *AnimBlueprintPath))
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			MeshComponent->SetAnimInstanceClass(AnimClass);
			UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] 已应用大厅展示动画蓝图：%s"), *AnimBlueprintPath);
			return true;
		}
	}

	const FString IdleAnimationPath = IsRosalesMeshPath(MeshPath)
		? FString(TEXT("/Game/DBA/Characters/Rosales/Animations/AN_Standing_Idle.AN_Standing_Idle"))
		: GetPreviewIdleAnimationPathForZodiac(Zodiac);
	if (!IdleAnimationPath.IsEmpty())
	{
		if (UAnimationAsset* IdleAnimation = LoadObject<UAnimationAsset>(nullptr, *IdleAnimationPath))
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			MeshComponent->SetAnimation(IdleAnimation);
			MeshComponent->Play(true);
			UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPresentationActor] 动画蓝图缺失，使用待机动画兜底：%s"), *IdleAnimationPath);
			return true;
		}
	}

	UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPresentationActor] 解析大厅展示动画失败。网格=%s 动画蓝图=%s"), *MeshPath, *AnimBlueprintPath);
	return false;
}

bool ADBACharacterPresentationActor::ApplyZodiacMaterialToMesh(USkeletalMeshComponent* MeshComponent, EDBAZodiac Zodiac, UObject* Outer)
{
	if (!MeshComponent)
	{
		return false;
	}

	const FString MaterialPath = GetPreviewMaterialPathForZodiac(Zodiac);
	UMaterialInterface* LoadedZodiacMaterial = MaterialPath.IsEmpty()
		? nullptr
		: LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
	const FString MeshPath = MeshComponent->GetSkeletalMeshAsset()
		? MeshComponent->GetSkeletalMeshAsset()->GetPathName()
		: FString();
	const bool bForceReliableTintMaterial = MeshPath.Contains(TEXT("/Game/DBA/Characters/Rosales/"));
	UMaterialInterface* BaseMaterial = bForceReliableTintMaterial ? LoadReliableFallbackMaterial() : LoadedZodiacMaterial;
	bool bUsedFallbackMaterial = bForceReliableTintMaterial && BaseMaterial != nullptr;
	if (!BaseMaterial || IsDefaultErrorMaterial(BaseMaterial))
	{
		// First fallback for imported meshes: tint the mesh's own material when it is valid.
		BaseMaterial = MeshComponent->GetMaterial(0);
		if (!BaseMaterial || IsDefaultErrorMaterial(BaseMaterial))
		{
			BaseMaterial = LoadReliableFallbackMaterial();
			bUsedFallbackMaterial = BaseMaterial != nullptr;
		}
		if (!BaseMaterial)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPresentationActor] 解析基础材质或兜底材质失败。生肖材质=%s"), *MaterialPath);
			return false;
		}
	}

	UMaterialInterface* MaterialToApply = BaseMaterial;
	const int32 MaterialSlotCount = FMath::Max(1, MeshComponent->GetNumMaterials());
	const FLinearColor Tint = GetPreviewTintForZodiac(Zodiac);
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
	{
		MaterialToApply = BaseMaterial;
		if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Outer ? Outer : MeshComponent))
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Tint"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("BodyColor"), Tint);
			DynamicMaterial->SetVectorParameterValue(TEXT("PrimaryColor"), Tint);

			if (bForceReliableTintMaterial)
			{
				const TCHAR* TexturePath = MaterialIndex == 0
					? TEXT("/Game/DBA/Characters/Rosales/Meshes/T_Rosales_Diffuse.T_Rosales_Diffuse")
					: TEXT("/Game/DBA/Characters/Rosales/Meshes/T_Rosales_Diffuse_Body.T_Rosales_Diffuse_Body");
				if (UTexture* AlbedoTexture = LoadObject<UTexture>(nullptr, TexturePath))
				{
					DynamicMaterial->SetTextureParameterValue(TEXT("AlbedoTexture"), AlbedoTexture);
					DynamicMaterial->SetTextureParameterValue(TEXT("BaseTexture"), AlbedoTexture);
					DynamicMaterial->SetTextureParameterValue(TEXT("DiffuseTexture"), AlbedoTexture);
				}
			}

			MaterialToApply = DynamicMaterial;
		}
		MeshComponent->SetMaterial(MaterialIndex, MaterialToApply);
	}

	UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] 已应用生肖材质：材质=%s 颜色=%s 槽位=%d 父材质=%s 纹理染色=%s"),
		LoadedZodiacMaterial && !bUsedFallbackMaterial ? *MaterialPath : TEXT("ReliableTintFallback"),
		*Tint.ToString(),
		MaterialSlotCount,
		*BaseMaterial->GetPathName(),
		bForceReliableTintMaterial ? TEXT("RosalesAlbedo") : TEXT("Default"));
	return true;
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
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] 已激活展示摄像机。Actor=%s 摄像机=%s 位置=%s"),
			*GetName(),
			PresentationCamera && PresentationCamera->IsActive() ? TEXT("Active") : TEXT("Inactive"),
			*GetActorLocation().ToString());
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPresentationActor] 激活展示摄像机失败：没有 PlayerController。"));
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

	ApplyStageSurfaceMaterial(GroundPlane, this, FLinearColor(0.015f, 0.020f, 0.028f, 1.0f), 0.34f, 0.55f, 1.75f);
	ApplyStageSurfaceMaterial(Pedestal, this, FLinearColor(0.020f, 0.025f, 0.036f, 1.0f), 0.34f, 0.55f, 1.5f);
	ApplyStageSurfaceMaterial(BackdropPlane, this, FLinearColor(0.010f, 0.018f, 0.032f, 1.0f), 0.6f, 0.25f, 1.0f);
	ApplyStageSurfaceMaterial(LeftPillar, this, FLinearColor(0.18f, 0.035f, 0.020f, 1.0f), 0.38f, 0.55f, 1.4f, 0.25f);
	ApplyStageSurfaceMaterial(RightPillar, this, FLinearColor(0.18f, 0.035f, 0.020f, 1.0f), 0.38f, 0.55f, 1.4f, 0.25f);
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
		KeyLight->SetCastShadows(true);
		KeyLight->SetUseTemperature(true);
		KeyLight->SetTemperature(10000.0f);
		KeyLight->SetLightSourceAngle(2.0f);
		KeyLight->ContactShadowLength = 0.2f;
		KeyLight->ContactShadowCastingIntensity = 1.0f;
		KeyLight->SetVolumetricScatteringIntensity(1.0f);
	}

	if (FillLight)
	{
		FillLight->SetRelativeRotation(FRotator(-14.0f, 146.0f, 0.0f));
		FillLight->SetIntensity(Spec.FillLightIntensity);
		FillLight->SetVisibility(false);
		FillLight->SetHiddenInGame(true);
	}

	if (RimLight)
	{
		RimLight->SetRelativeRotation(FRotator(-18.0f, 218.0f, 0.0f));
		RimLight->SetIntensity(Spec.RimLightIntensity);
		RimLight->SetVisibility(false);
		RimLight->SetHiddenInGame(true);
	}

	if (FaceLight)
	{
		FaceLight->SetRelativeLocation(FVector(205.0f, 0.0f, 126.0f));
		FaceLight->SetIntensity(Spec.FaceLightIntensity);
		FaceLight->SetAttenuationRadius(220.0f);
		FaceLight->SetSourceRadius(30.0f);
		FaceLight->SetSoftSourceRadius(30.0f);
		FaceLight->SetLightColor(FLinearColor(0.25f, 0.45f, 1.0f));
		FaceLight->SetCastShadows(false);
		FaceLight->SetVolumetricScatteringIntensity(0.3f);
	}

	if (LeftPillarRedLight)
	{
		LeftPillarRedLight->SetRelativeLocation(FVector(-112.0f, -150.0f, 112.0f));
		LeftPillarRedLight->SetIntensity(Spec.PillarRedLightIntensity);
		LeftPillarRedLight->SetAttenuationRadius(450.0f);
		LeftPillarRedLight->SetSourceRadius(40.0f);
		LeftPillarRedLight->SetSoftSourceRadius(100.0f);
		LeftPillarRedLight->SetLightColor(FLinearColor(1.0f, 0.23f, 0.08f));
		LeftPillarRedLight->SetCastShadows(true);
		LeftPillarRedLight->SetVolumetricScatteringIntensity(2.5f);
	}

	if (RightPillarRedLight)
	{
		RightPillarRedLight->SetRelativeLocation(FVector(-112.0f, 150.0f, 112.0f));
		RightPillarRedLight->SetIntensity(Spec.PillarRedLightIntensity);
		RightPillarRedLight->SetAttenuationRadius(450.0f);
		RightPillarRedLight->SetSourceRadius(40.0f);
		RightPillarRedLight->SetSoftSourceRadius(100.0f);
		RightPillarRedLight->SetLightColor(FLinearColor(1.0f, 0.23f, 0.08f));
		RightPillarRedLight->SetCastShadows(true);
		RightPillarRedLight->SetVolumetricScatteringIntensity(2.5f);
	}

	if (SkyLight)
	{
		SkyLight->SetIntensity(Spec.SkyLightIntensity);
		SkyLight->SetLightColor(FLinearColor(0.35f, 0.45f, 0.65f));
		SkyLight->SetLowerHemisphereColor(FLinearColor(0.005f, 0.008f, 0.015f));
		SkyLight->bLowerHemisphereIsBlack = true;
		SkyLight->SetRealTimeCapture(true);
	}

	if (PostProcess)
	{
		PostProcess->SetRelativeLocation(FVector(120.0f, 0.0f, 96.0f));
		PostProcess->bUnbound = true;
		FPostProcessSettings& PPS = PostProcess->Settings;
		PPS.bOverride_AutoExposureMethod = true;
		PPS.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
		PPS.bOverride_AutoExposureMinBrightness = true;
		PPS.AutoExposureMinBrightness = -1.5f;
		PPS.bOverride_AutoExposureMaxBrightness = true;
		PPS.AutoExposureMaxBrightness = -1.5f;
		PPS.bOverride_AutoExposureBias = true;
		PPS.AutoExposureBias = -0.8f;
		PPS.bOverride_WhiteTemp = true;
		PPS.WhiteTemp = 10000.0f;
		PPS.bOverride_WhiteTint = true;
		PPS.WhiteTint = -0.18f;
		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = FVector4(0.82f, 0.82f, 0.82f, 1.0f);
		PPS.bOverride_ColorContrast = true;
		PPS.ColorContrast = FVector4(1.18f, 1.18f, 1.18f, 1.0f);
		PPS.bOverride_ColorGamma = true;
		PPS.ColorGamma = FVector4(0.95f, 0.95f, 0.95f, 1.0f);
		PPS.bOverride_ColorGain = true;
		PPS.ColorGain = FVector4(0.92f, 0.92f, 0.92f, 1.0f);
		PPS.bOverride_BloomIntensity = true;
		PPS.BloomIntensity = 0.25f;
		PPS.bOverride_BloomThreshold = true;
		PPS.BloomThreshold = 1.5f;
		PPS.bOverride_VignetteIntensity = true;
		PPS.VignetteIntensity = 0.45f;
		PPS.bOverride_FilmGrainIntensity = true;
		PPS.FilmGrainIntensity = 0.08f;
	}

	if (AtmosphereFog)
	{
		AtmosphereFog->SetVisibility(Spec.bUseAtmosphericFog);
		AtmosphereFog->SetHiddenInGame(!Spec.bUseAtmosphericFog);
		AtmosphereFog->SetFogDensity(0.028f);
		AtmosphereFog->SetFogHeightFalloff(0.35f);
		AtmosphereFog->SetFogInscatteringColor(FLinearColor(0.03f, 0.06f, 0.10f, 1.0f));
		AtmosphereFog->SetDirectionalInscatteringColor(FLinearColor(0.12f, 0.22f, 0.35f, 1.0f));
		AtmosphereFog->SetDirectionalInscatteringExponent(12.0f);
		AtmosphereFog->SetStartDistance(72.0f);
		AtmosphereFog->SetVolumetricFog(true);
		AtmosphereFog->SetVolumetricFogScatteringDistribution(0.35f);
		AtmosphereFog->SetVolumetricFogAlbedo(FLinearColor(0.25f, 0.35f, 0.50f, 1.0f).ToFColor(false));
		AtmosphereFog->SetVolumetricFogExtinctionScale(1.0f);
		AtmosphereFog->SetVolumetricFogDistance(6000.0f);
	}
}

void ADBACharacterPresentationActor::ApplyPreviewAssets(EDBAZodiac Zodiac)
{
	if (!PreviewMeshComponent)
	{
		return;
	}

	const TArray<FString> MeshCandidates = GetLobbyDisplayMeshCandidatePathsForZodiac(Zodiac);

	USkeletalMesh* ResolvedMesh = nullptr;
	FString ResolvedMeshPath;
	USkeletalMesh* FirstLoadedMesh = nullptr;
	FString FirstLoadedMeshPath;
	for (const FString& MeshPath : MeshCandidates)
	{
		if (!MeshPath.IsEmpty())
		{
			if (USkeletalMesh* CandidateMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath))
			{
				EnsureRosalesMeshUsesRosalesSkeleton(CandidateMesh, MeshPath);
				if (!FirstLoadedMesh)
				{
					FirstLoadedMesh = CandidateMesh;
					FirstLoadedMeshPath = MeshPath;
				}
				if (CandidateMesh->GetSkeleton())
				{
					ResolvedMesh = CandidateMesh;
					ResolvedMeshPath = MeshPath;
					break;
				}
			}
		}
	}

	if (!ResolvedMesh)
	{
		ResolvedMesh = FirstLoadedMesh;
		ResolvedMeshPath = FirstLoadedMeshPath;
	}

	if (!ResolvedMesh)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[CharacterPresentationActor] 加载预览骨骼网格失败。"));
		return;
	}
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] 已加载网格：%s 骨骼=%s"),
		*ResolvedMeshPath,
		ResolvedMesh->GetSkeleton() ? TEXT("Valid") : TEXT("None"));

	PreviewMeshComponent->SetSkeletalMesh(ResolvedMesh);
	PreviewMeshComponent->SetRelativeRotation(GetPreviewMeshPlayerFacingRotation());
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
	const FLinearColor ZodiacTint = GetPreviewTintForZodiac(Zodiac);
	if (FaceLight)
	{
		FaceLight->SetLightColor(ZodiacTint);
		FaceLight->SetIntensity(GetReferenceStageSpec().FaceLightIntensity);
	}
	if (RimLight)
	{
		RimLight->SetLightColor(ZodiacTint);
	}
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] 网格已应用。缩放=%.2f Z偏移=%.2f 包围盒=%s"),
		PreviewMeshDisplayScale,
		MeshBottomOffsetZ,
		*PreviewMeshComponent->Bounds.GetBox().ToString());

	if (!ApplyLobbyDisplayAnimationToMesh(PreviewMeshComponent, ResolvedMeshPath, Zodiac))
	{
		PreviewMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}

	ApplyZodiacMaterialToMesh(PreviewMeshComponent, Zodiac, this);
}
