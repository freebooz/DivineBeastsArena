// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Frontend/CharacterSelection/DBACharacterPresentationActor.h"

#include "GameDBA/Data/Tables/DBAZodiacPlaceholderTintRow.h"
#include "GameDBA/Presentation/Visual/DBAZodiacVisualDeveloperSettings.h"
#include "GameDBA/Data/Registries/DBAZodiacCharacterRegistry.h"
#include "GameDBA/Character/Appearance/DBACharacterAppearanceComponent.h"
#include "GameDBA/Frontend/DBAFrontendEnvironmentSubsystem.h"
#include "Animation/AnimationAsset.h"
#include "Animation/Skeleton.h"
#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Scene.h"
#include "Engine/Texture.h"
#include "EngineUtils.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameFramework/PlayerController.h"
#include "Animation/AnimInstance.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	constexpr float PreviewMeshDisplayScale = 1.0f;
	constexpr float PreviewMeshFloorZ = 2.0f;

	template <typename TObjectType>
	TObjectType* ResolveSoftObjectSync(const TSoftObjectPtr<TObjectType>& SoftRef)
	{
		if (TObjectType* LoadedObject = SoftRef.Get())
		{
			return LoadedObject;
		}

		return Cast<TObjectType>(SoftRef.ToSoftObjectPath().TryLoad());
	}

	template <typename TObjectType>
	TObjectType* ResolveSoftObjectSync(const FString& ObjectPath)
	{
		if (ObjectPath.IsEmpty())
		{
			return nullptr;
		}

		return ResolveSoftObjectSync(TSoftObjectPtr<TObjectType>(FSoftObjectPath(ObjectPath)));
	}

	bool TryGetZodiacPresentationDefinition(
		EDBAZodiac Zodiac,
		FDBAZodiacCharacterPresentationDefinition& OutDefinition)
	{
		const UDBAZodiacVisualDeveloperSettings* VisualSettings = GetDefault<UDBAZodiacVisualDeveloperSettings>();
		if (!VisualSettings || VisualSettings->ZodiacCharacterRegistry.IsNull())
		{
			UE_LOG(LogDBAData, Error, TEXT("[生肖角色展示] 未配置生肖角色注册表数据资产。"));
			return false;
		}

		const UDBAZodiacCharacterRegistry* Registry = VisualSettings->ZodiacCharacterRegistry.LoadSynchronous();
		return Registry && Registry->GetPresentationDefinitionForZodiac(Zodiac, OutDefinition);
	}

	void EnsureRosalesMeshUsesRosalesSkeleton(USkeletalMesh* Mesh, const FString& MeshPath)
	{
		if (!Mesh || !MeshPath.Contains(TEXT("/Game/DBA/Characters/Rosales/")))
		{
			return;
		}

		// P1-6 阶段 B 改造：优先 Get()，未加载时 TryLoad() 同步兜底，避免展示舞台网格为空。
		static const TSoftObjectPtr<USkeleton> RosalesSkeletonRef(FSoftObjectPath(TEXT("/Game/DBA/Characters/Rosales/Meshes/SKEL_Rosales.SKEL_Rosales")));
		if (USkeleton* RosalesSkeleton = ResolveSoftObjectSync(RosalesSkeletonRef))
		{
			Mesh->SetSkeleton(RosalesSkeleton);
		}
		else
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPresentationActor] Rosales 骨骼资源未加载，跳过骨骼设置。请确认资源已预加载。"));
		}
	}

	UMaterialInterface* LoadReliableFallbackMaterial()
	{
		// P1-6 阶段 B 改造：优先 Get()，未加载时 TryLoad() 同步兜底。
		static const TSoftObjectPtr<UMaterialInterface> RuntimeTintMaterialRef(FSoftObjectPath(TEXT("/Game/DBA/Materials/M_DBA_RuntimeTint.M_DBA_RuntimeTint")));
		if (UMaterialInterface* RuntimeTintMaterial = ResolveSoftObjectSync(RuntimeTintMaterialRef))
		{
			return RuntimeTintMaterial;
		}
		static const TSoftObjectPtr<UMaterialInterface> BasicShapeMaterialRef(FSoftObjectPath(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")));
		return ResolveSoftObjectSync(BasicShapeMaterialRef);
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

	static FLinearColor ResolveDefaultPreviewTintForZodiac(EDBAZodiac Zodiac)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[CharacterPresentationActor] 十二生肖染色数据表不可用，无法解析生肖=%d 的外观颜色。"), static_cast<int32>(Zodiac));
		return FLinearColor::White;
	}

	const UDBAZodiacVisualDeveloperSettings* GetZodiacVisualSettings()
	{
		return GetDefault<UDBAZodiacVisualDeveloperSettings>();
	}

	FName GetZodiacRowName(EDBAZodiac Zodiac)
	{
		if (const UEnum* ZodiacEnum = StaticEnum<EDBAZodiac>())
		{
			return FName(*ZodiacEnum->GetNameStringByValue(static_cast<int64>(Zodiac)));
		}
		return NAME_None;
	}

	bool TryResolvePlaceholderTintRow(EDBAZodiac Zodiac, FDBAZodiacPlaceholderTintRow& OutRow)
	{
		const UDBAZodiacVisualDeveloperSettings* Settings = GetZodiacVisualSettings();
		if (!Settings || Zodiac == EDBAZodiac::None)
		{
			return false;
		}

		const TSoftObjectPtr<UDataTable>& TintTablePtr = Settings->ZodiacPlaceholderTintTable;
		if (!TintTablePtr.ToSoftObjectPath().IsValid())
		{
			return false;
		}

		UDataTable* TintTable = TintTablePtr.Get();
		if (!TintTable)
		{
			TintTable = Cast<UDataTable>(TintTablePtr.ToSoftObjectPath().TryLoad());
		}
		if (!TintTable)
		{
			return false;
		}

		const FName RowName = GetZodiacRowName(Zodiac);
		if (const FDBAZodiacPlaceholderTintRow* Row = TintTable->FindRow<FDBAZodiacPlaceholderTintRow>(RowName, TEXT("ZodiacPlaceholderTint")))
		{
			OutRow = *Row;
			return true;
		}

		return false;
	}

	FString GetConfiguredPlaceholderMeshPath()
	{
		if (const UDBAZodiacVisualDeveloperSettings* Settings = GetZodiacVisualSettings())
		{
			const FString ConfiguredPath = Settings->PlaceholderSkeletalMesh.ToSoftObjectPath().ToString();
			if (!ConfiguredPath.IsEmpty())
			{
				return ConfiguredPath;
			}
		}
		return TEXT("/Game/DBA/Characters/Rosales/Meshes/SK_Rosales.SK_Rosales");
	}

	UMaterialInterface* LoadConfiguredPlaceholderTintMaterial()
	{
		if (const UDBAZodiacVisualDeveloperSettings* Settings = GetZodiacVisualSettings())
		{
			if (UMaterialInterface* ConfiguredMaterial = Settings->PlaceholderTintMaterial.Get())
			{
				return ConfiguredMaterial;
			}
			if (UMaterialInterface* LoadedMaterial = Cast<UMaterialInterface>(Settings->PlaceholderTintMaterial.ToSoftObjectPath().TryLoad()))
			{
				return LoadedMaterial;
			}
		}
		return LoadReliableFallbackMaterial();
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

	AppearanceComponent = CreateDefaultSubobject<UDBACharacterAppearanceComponent>(TEXT("AppearanceComponent"));
	AppearanceComponent->SetBaseMeshComponent(PreviewMeshComponent);

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
	KeyLight->SetAtmosphereSunLight(true);
	KeyLight->SetAtmosphereSunLightIndex(0);
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

	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(StageRoot);
	SkyAtmosphere->SetMobility(EComponentMobility::Movable);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(StageRoot);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->SetCastShadows(false);
	SkyLight->SetLightColor(FLinearColor(0.35f, 0.45f, 0.65f));
	SkyLight->SetLowerHemisphereColor(FLinearColor(0.005f, 0.008f, 0.015f));
	SkyLight->bLowerHemisphereIsBlack = true;
	// 与 SkyAtmosphere 配套启用实时捕获，满足引擎对天空光实时捕获的组件要求。
	SkyLight->SetRealTimeCapture(true);
	SkyLight->SourceType = ESkyLightSourceType::SLS_CapturedScene;

	AtmosphereFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("AtmosphereFog"));
	AtmosphereFog->SetupAttachment(StageRoot);
	AtmosphereFog->SetMobility(EComponentMobility::Movable);

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(StageRoot);
	PostProcess->SetMobility(EComponentMobility::Movable);
	PostProcess->bUnbound = true;
	PostProcess->BlendWeight = 1.0f;

	ApplyStageSpec();
}

FDBACharacterPresentationStageSpec ADBACharacterPresentationActor::GetReferenceStageSpec()
{
	return FDBACharacterPresentationStageSpec();
}

ADBACharacterPresentationActor* ADBACharacterPresentationActor::FindPlacedPresentationStage(UWorld* World)
{
	if (!World)
	{
		UE_LOG(LogDBACore, Error, TEXT("[角色展示舞台] 当前世界无效，无法查找固定展示舞台。"));
		return nullptr;
	}

	for (TActorIterator<ADBACharacterPresentationActor> It(World); It; ++It)
	{
		if (IsValid(*It))
		{
			return *It;
		}
	}

	UE_LOG(LogDBACore, Error, TEXT("[角色展示舞台] 当前关卡未放置角色展示舞台。请在 FrontendMap 中放置 ADBACharacterPresentationActor。"));
	return nullptr;
}

FString ADBACharacterPresentationActor::GetPreviewMeshPathForZodiac(EDBAZodiac Zodiac)
{
	FDBAZodiacCharacterPresentationDefinition Definition;
	return TryGetZodiacPresentationDefinition(Zodiac, Definition)
		? Definition.SkeletalMesh.ToSoftObjectPath().ToString()
		: FString();
}

TArray<FString> ADBACharacterPresentationActor::GetLobbyDisplayMeshCandidatePathsForZodiac(EDBAZodiac Zodiac)
{
	TArray<FString> Candidates;
	const FString MeshPath = GetPreviewMeshPathForZodiac(Zodiac);
	if (!MeshPath.IsEmpty())
	{
		Candidates.Add(MeshPath);
	}
	return Candidates;
}

FString ADBACharacterPresentationActor::GetLobbyDisplayAnimBlueprintPathForMeshPath(const FString& MeshPath, EDBAZodiac Zodiac)
{
	FDBAZodiacCharacterPresentationDefinition Definition;
	return TryGetZodiacPresentationDefinition(Zodiac, Definition)
		? Definition.AnimationBlueprintClass.ToSoftObjectPath().ToString()
		: FString();
}

FString ADBACharacterPresentationActor::GetPreviewIdleAnimationPathForZodiac(EDBAZodiac Zodiac)
{
	FDBAZodiacCharacterPresentationDefinition Definition;
	return TryGetZodiacPresentationDefinition(Zodiac, Definition)
		? Definition.IdleAnimation.ToSoftObjectPath().ToString()
		: FString();
}

FString ADBACharacterPresentationActor::GetPreviewMaterialPathForZodiac(EDBAZodiac Zodiac)
{
	FDBAZodiacCharacterPresentationDefinition Definition;
	return TryGetZodiacPresentationDefinition(Zodiac, Definition)
		? Definition.Material.ToSoftObjectPath().ToString()
		: FString();
}

FLinearColor ADBACharacterPresentationActor::GetPreviewTintForZodiac(EDBAZodiac Zodiac)
{
	FDBAZodiacPlaceholderTintRow TintRow;
	if (TryResolvePlaceholderTintRow(Zodiac, TintRow))
	{
		return TintRow.BodyTint;
	}

	return ResolveDefaultPreviewTintForZodiac(Zodiac);
}

FLinearColor ADBACharacterPresentationActor::GetPreviewAccentTintForZodiac(EDBAZodiac Zodiac)
{
	FDBAZodiacPlaceholderTintRow TintRow;
	if (TryResolvePlaceholderTintRow(Zodiac, TintRow))
	{
		return TintRow.AccentTint;
	}

	return GetPreviewTintForZodiac(Zodiac);
}

bool ADBACharacterPresentationActor::UsesTintedPlaceholderMesh()
{
	const UDBAZodiacVisualDeveloperSettings* Settings = GetDefault<UDBAZodiacVisualDeveloperSettings>();
	return Settings ? Settings->bUseTintedPlaceholderMesh : true;
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

	const FString AnimBlueprintPath = GetLobbyDisplayAnimBlueprintPathForMeshPath(MeshPath, Zodiac);
	if (!AnimBlueprintPath.IsEmpty())
	{
		// 动画蓝图类加载保留 LoadClass（引擎内置资源在启动时已加载，TSoftObjectPtr 无法直接处理 UClass）
		if (UClass* AnimClass = LoadClass<UAnimInstance>(nullptr, *AnimBlueprintPath))
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			MeshComponent->SetAnimInstanceClass(AnimClass);
			UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] 已应用大厅展示动画蓝图：%s"), *AnimBlueprintPath);
			return true;
		}
	}

	const FString IdleAnimationPath = GetPreviewIdleAnimationPathForZodiac(Zodiac);
	if (!IdleAnimationPath.IsEmpty())
	{
		const FSoftObjectPath IdleAnimSoftPath(IdleAnimationPath);
		const TSoftObjectPtr<UAnimationAsset> IdleAnimRef(IdleAnimSoftPath);
		if (UAnimationAsset* IdleAnimation = ResolveSoftObjectSync(IdleAnimRef))
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
	// P1-6 阶段 B 改造：使用 TSoftObjectPtr 软引用替代 LoadObject 同步加载
	const TSoftObjectPtr<UMaterialInterface> ZodiacMaterialRef(MaterialPath.IsEmpty() ? FSoftObjectPath() : FSoftObjectPath(MaterialPath));
	UMaterialInterface* LoadedZodiacMaterial = MaterialPath.IsEmpty() ? nullptr : ResolveSoftObjectSync(ZodiacMaterialRef);
	UMaterialInterface* BaseMaterial = LoadedZodiacMaterial;
	if (!BaseMaterial || IsDefaultErrorMaterial(BaseMaterial))
	{
		// 不使用其他角色或引擎路径回退；仅接受注册表材质或当前网格自带材质。
		BaseMaterial = MeshComponent->GetMaterial(0);
		if (!BaseMaterial)
		{
			UE_LOG(LogDBAUI, Error, TEXT("[CharacterPresentationActor] 未配置有效生肖材质。生肖材质=%s"), *MaterialPath);
			return false;
		}
	}

	UMaterialInterface* MaterialToApply = BaseMaterial;
	const int32 MaterialSlotCount = FMath::Max(1, MeshComponent->GetNumMaterials());
	const FLinearColor Tint = GetPreviewTintForZodiac(Zodiac);
	const FLinearColor AccentTint = GetPreviewAccentTintForZodiac(Zodiac);
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
	{
		MaterialToApply = BaseMaterial;
		if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Outer ? Outer : MeshComponent))
		{
			const FLinearColor SlotTint = (MaterialIndex == 0) ? Tint : AccentTint;
			DynamicMaterial->SetVectorParameterValue(TEXT("Tint"), SlotTint);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), SlotTint);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), SlotTint);
			DynamicMaterial->SetVectorParameterValue(TEXT("BodyColor"), SlotTint);
			DynamicMaterial->SetVectorParameterValue(TEXT("PrimaryColor"), SlotTint);
			DynamicMaterial->SetVectorParameterValue(TEXT("AccentColor"), AccentTint);
			DynamicMaterial->SetVectorParameterValue(TEXT("SecondaryColor"), AccentTint);

			MaterialToApply = DynamicMaterial;
		}
		MeshComponent->SetMaterial(MaterialIndex, MaterialToApply);
	}

	UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] 已应用生肖材质：材质=%s 颜色=%s 槽位=%d 父材质=%s 纹理染色=%s"),
		LoadedZodiacMaterial ? *MaterialPath : TEXT("网格自带材质"),
		*Tint.ToString(),
		MaterialSlotCount,
		*BaseMaterial->GetPathName(),
		TEXT("注册表"));
	return true;
}

void ADBACharacterPresentationActor::BeginPlay()
{
	Super::BeginPlay();
}

void ADBACharacterPresentationActor::SetPreviewZodiac(EDBAZodiac Zodiac)
{
	if (Zodiac == EDBAZodiac::None)
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPresentationActor] 未选择生肖，跳过预览资源加载。"));
		return;
	}

	ApplyPreviewAssets(Zodiac);
}

void ADBACharacterPresentationActor::ApplyPreviewAppearance(const EDBAZodiac Zodiac, const FDBACharacterAppearance& Appearance)
{
	SetPreviewZodiac(Zodiac);
	if (AppearanceComponent)
	{
		AppearanceComponent->SetBaseMeshComponent(PreviewMeshComponent);
		AppearanceComponent->ApplyAppearance(Zodiac, Appearance);
	}
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
		SetActorHiddenInGame(false);
		SetActorTickEnabled(true);
		SetActorEnableCollision(false);

		auto ForceStageComponentVisible = [](USceneComponent* Component)
		{
			if (!Component)
			{
				return;
			}
			Component->SetVisibility(true, true);
			Component->SetHiddenInGame(false, true);
		};
		ForceStageComponentVisible(PreviewMeshComponent);
		ForceStageComponentVisible(GroundPlane);
		ForceStageComponentVisible(Pedestal);
		ForceStageComponentVisible(BackdropPlane);
		ForceStageComponentVisible(LeftPillar);
		ForceStageComponentVisible(RightPillar);
		ForceStageComponentVisible(MoonDisc);
		ForceStageComponentVisible(KeyLight);
		ForceStageComponentVisible(FillLight);
		ForceStageComponentVisible(RimLight);
		ForceStageComponentVisible(FaceLight);
		ForceStageComponentVisible(LeftPillarRedLight);
		ForceStageComponentVisible(RightPillarRedLight);
		ForceStageComponentVisible(SkyLight);
		ForceStageComponentVisible(SkyAtmosphere);
		ForceStageComponentVisible(AtmosphereFog);
		ForceStageComponentVisible(PostProcess);

		if (UWorld* World = GetWorld())
		{
			if (UDBAFrontendEnvironmentSubsystem* FrontendEnvironment = World->GetSubsystem<UDBAFrontendEnvironmentSubsystem>())
			{
				FrontendEnvironment->EnableCharacterPresentationRendering();
			}

			// 再保险：关卡后处理体积即使 Hidden 仍会被合成，激活舞台时一律关闭。
			for (TActorIterator<APostProcessVolume> It(World); It; ++It)
			{
				if (APostProcessVolume* Volume = *It)
				{
					Volume->bEnabled = false;
					Volume->BlendWeight = 0.0f;
				}
			}
		}

		if (SkyLight)
		{
			SkyLight->RecaptureSky();
		}

		if (PresentationCamera)
		{
			PresentationCamera->Activate(true);
			PresentationCamera->SetActive(true, true);
			PresentationCamera->PostProcessBlendWeight = 0.0f;
		}

		// 禁用自动相机管理，避免大厅 Pawn/旧 ViewTarget 抢回导致只剩黑底 UI。
		PlayerController->bAutoManageActiveCameraTarget = false;
		if (BlendTime <= KINDA_SMALL_NUMBER)
		{
			PlayerController->SetViewTarget(this);
		}
		else
		{
			PlayerController->SetViewTargetWithBlend(this, BlendTime);
		}

		if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
		{
			CameraManager->bDefaultConstrainAspectRatio = false;
			CameraManager->SetViewTarget(this);
		}
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] 已激活展示摄像机与背景舞台。Actor=%s 摄像机=%s 位置=%s 地面=%s 网格=%s"),
			*GetName(),
			PresentationCamera && PresentationCamera->IsActive() ? TEXT("Active") : TEXT("Inactive"),
			*GetActorLocation().ToString(),
			GroundPlane && GroundPlane->GetStaticMesh() ? TEXT("有") : TEXT("无"),
			PreviewMeshComponent && PreviewMeshComponent->GetSkeletalMeshAsset() ? TEXT("有") : TEXT("无"));
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPresentationActor] 激活展示摄像机失败：没有 PlayerController。"));
	}
}

void ADBACharacterPresentationActor::ConfigureStageVisuals()
{
	// 引擎 BasicShape 可能尚未驻留内存：优先 Get()，失败时 TryLoad() 同步兜底，确保选角背景舞台可见。
	UStaticMesh* PlaneMesh = ResolveSoftObjectSync<UStaticMesh>(FString(TEXT("/Engine/BasicShapes/Plane.Plane")));
	UStaticMesh* CylinderMesh = ResolveSoftObjectSync<UStaticMesh>(FString(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
	UStaticMesh* CubeMesh = ResolveSoftObjectSync<UStaticMesh>(FString(TEXT("/Engine/BasicShapes/Cube.Cube")));
	UStaticMesh* SphereMesh = ResolveSoftObjectSync<UStaticMesh>(FString(TEXT("/Engine/BasicShapes/Sphere.Sphere")));

	auto ConfigureStageMesh = [](UStaticMeshComponent* MeshComponent, UStaticMesh* Mesh)
	{
		if (!MeshComponent || !Mesh)
		{
			return;
		}
		MeshComponent->SetStaticMesh(Mesh);
		MeshComponent->SetVisibility(true);
		MeshComponent->SetHiddenInGame(false);
		MeshComponent->SetCastShadow(true);
	};

	ConfigureStageMesh(GroundPlane, PlaneMesh);
	ConfigureStageMesh(BackdropPlane, PlaneMesh);
	ConfigureStageMesh(Pedestal, CylinderMesh);
	ConfigureStageMesh(LeftPillar, CubeMesh);
	ConfigureStageMesh(RightPillar, CubeMesh);
	ConfigureStageMesh(MoonDisc, SphereMesh ? SphereMesh : PlaneMesh);

	if (!PlaneMesh || !CylinderMesh || !CubeMesh)
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterPresentationActor] 舞台背景网格加载不完整：地面=%s 台座=%s 立柱=%s"),
			PlaneMesh ? TEXT("是") : TEXT("否"),
			CylinderMesh ? TEXT("是") : TEXT("否"),
			CubeMesh ? TEXT("是") : TEXT("否"));
	}
	else
	{
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterPresentationActor] 已装配角色选择背景舞台网格。"));
	}

	ApplyStageSurfaceMaterial(GroundPlane, this, FLinearColor(0.045f, 0.055f, 0.075f, 1.0f), 0.42f, 0.45f, 1.35f);
	ApplyStageSurfaceMaterial(Pedestal, this, FLinearColor(0.08f, 0.09f, 0.12f, 1.0f), 0.38f, 0.55f, 1.4f, 0.08f);
	ApplyStageSurfaceMaterial(BackdropPlane, this, FLinearColor(0.04f, 0.07f, 0.14f, 1.0f), 0.72f, 0.18f, 0.8f, 0.12f);
	ApplyStageSurfaceMaterial(LeftPillar, this, FLinearColor(0.42f, 0.08f, 0.04f, 1.0f), 0.38f, 0.55f, 1.4f, 0.45f);
	ApplyStageSurfaceMaterial(RightPillar, this, FLinearColor(0.42f, 0.08f, 0.04f, 1.0f), 0.38f, 0.55f, 1.4f, 0.45f);
	ApplyStageSurfaceMaterial(MoonDisc, this, FLinearColor(0.72f, 0.82f, 1.0f, 1.0f), 0.55f, 0.2f, 0.6f, 1.8f);
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
		KeyLight->SetVisibility(true);
		KeyLight->SetHiddenInGame(false);
		KeyLight->SetCastShadows(true);
		KeyLight->SetUseTemperature(true);
		KeyLight->SetTemperature(6500.0f);
		KeyLight->SetLightSourceAngle(2.0f);
		KeyLight->ContactShadowLength = 0.2f;
		KeyLight->ContactShadowCastingIntensity = 1.0f;
		KeyLight->SetAtmosphereSunLight(true);
		KeyLight->SetAtmosphereSunLightIndex(0);
		KeyLight->SetVolumetricScatteringIntensity(1.0f);
	}

	if (FillLight)
	{
		const bool bEnableFill = Spec.FillLightIntensity > KINDA_SMALL_NUMBER;
		FillLight->SetRelativeRotation(FRotator(-14.0f, 146.0f, 0.0f));
		FillLight->SetIntensity(Spec.FillLightIntensity);
		FillLight->SetVisibility(bEnableFill);
		FillLight->SetHiddenInGame(!bEnableFill);
		FillLight->SetCastShadows(false);
	}

	if (RimLight)
	{
		const bool bEnableRim = Spec.RimLightIntensity > KINDA_SMALL_NUMBER;
		RimLight->SetRelativeRotation(FRotator(-18.0f, 218.0f, 0.0f));
		RimLight->SetIntensity(Spec.RimLightIntensity);
		RimLight->SetVisibility(bEnableRim);
		RimLight->SetHiddenInGame(!bEnableRim);
		RimLight->SetCastShadows(false);
	}

	if (FaceLight)
	{
		FaceLight->SetRelativeLocation(FVector(205.0f, 0.0f, 126.0f));
		FaceLight->SetIntensity(Spec.FaceLightIntensity);
		FaceLight->SetAttenuationRadius(420.0f);
		FaceLight->SetSourceRadius(45.0f);
		FaceLight->SetSoftSourceRadius(60.0f);
		FaceLight->SetLightColor(FLinearColor(0.45f, 0.62f, 1.0f));
		FaceLight->SetCastShadows(false);
		FaceLight->SetVisibility(true);
		FaceLight->SetHiddenInGame(false);
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

	if (SkyAtmosphere)
	{
		const bool bEnableAtmosphere = Spec.bUseSkyAtmosphere;
		SkyAtmosphere->SetVisibility(bEnableAtmosphere);
		SkyAtmosphere->SetHiddenInGame(!bEnableAtmosphere);
		SkyAtmosphere->SetMobility(EComponentMobility::Movable);
	}

	if (SkyLight)
	{
		SkyLight->SetIntensity(Spec.SkyLightIntensity);
		SkyLight->SetLightColor(FLinearColor(0.35f, 0.45f, 0.65f));
		SkyLight->SetLowerHemisphereColor(FLinearColor(0.005f, 0.008f, 0.015f));
		SkyLight->bLowerHemisphereIsBlack = true;
		SkyLight->SourceType = ESkyLightSourceType::SLS_CapturedScene;
		SkyLight->SetCubemap(nullptr);
		// 有 SkyAtmosphere 时启用实时捕获；否则退回静态捕获，避免引擎红字报错。
		SkyLight->SetRealTimeCapture(Spec.bUseSkyAtmosphere && SkyAtmosphere != nullptr);
		SkyLight->SetVisibility(true);
		SkyLight->SetHiddenInGame(false);
		SkyLight->RecaptureSky();
	}

	if (PostProcess)
	{
		PostProcess->SetRelativeLocation(FVector(120.0f, 0.0f, 96.0f));
		PostProcess->bUnbound = true;
		PostProcess->BlendWeight = 1.0f;
		FPostProcessSettings& PPS = PostProcess->Settings;
		// 角色创建场景使用自适应曝光，避免手动曝光在不同显卡与显示器上把角色压暗。
		PPS.bOverride_AutoExposureMethod = true;
		PPS.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
		PPS.bOverride_AutoExposureBias = true;
		PPS.AutoExposureBias = 1.5f;
		PPS.bOverride_AutoExposureMinBrightness = true;
		PPS.AutoExposureMinBrightness = 1.0f;
		PPS.bOverride_AutoExposureMaxBrightness = true;
		PPS.AutoExposureMaxBrightness = 3.0f;
		PPS.bOverride_WhiteTemp = true;
		PPS.WhiteTemp = 6500.0f;
		PPS.bOverride_WhiteTint = true;
		PPS.WhiteTint = 0.0f;
		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = FVector4(1.02f, 1.02f, 1.04f, 1.0f);
		PPS.bOverride_ColorContrast = true;
		PPS.ColorContrast = FVector4(1.02f, 1.02f, 1.02f, 1.0f);
		PPS.bOverride_ColorGamma = true;
		PPS.ColorGamma = FVector4(1.05f, 1.05f, 1.05f, 1.0f);
		PPS.bOverride_ColorGain = true;
		PPS.ColorGain = FVector4(1.35f, 1.35f, 1.38f, 1.0f);
		PPS.bOverride_BloomIntensity = true;
		PPS.BloomIntensity = 0.25f;
		PPS.bOverride_BloomThreshold = true;
		PPS.BloomThreshold = 1.1f;
		PPS.bOverride_VignetteIntensity = true;
		PPS.VignetteIntensity = 0.12f;
		PPS.bOverride_FilmGrainIntensity = true;
		PPS.FilmGrainIntensity = 0.0f;
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
			if (USkeletalMesh* CandidateMesh = ResolveSoftObjectSync<USkeletalMesh>(MeshPath))
			{
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
