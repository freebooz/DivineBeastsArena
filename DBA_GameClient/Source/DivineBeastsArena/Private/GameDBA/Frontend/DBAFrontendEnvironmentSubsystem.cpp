// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/DBAFrontendEnvironmentSubsystem.h"

#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/GameViewportClient.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterPresentationActor.h"
#include "Misc/CommandLine.h"
#include "TimerManager.h"

namespace DBAFrontendEnvironment
{
	bool IsSkyAtmosphereActor(const AActor* Actor)
	{
		return Actor && Actor->GetClass()->GetName().Contains(TEXT("SkyAtmosphere"));
	}

	bool IsVolumetricCloudActor(const AActor* Actor)
	{
		return Actor && Actor->GetClass()->GetName().Contains(TEXT("VolumetricCloud"));
	}

	bool IsWaterActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		const FString ClassName = Actor->GetClass()->GetName();
		return ClassName.Contains(TEXT("Water"))
			|| ClassName.Contains(TEXT("Ocean"))
			|| ClassName.Contains(TEXT("Buoyancy"));
	}
	bool IsFrontendWorld(const UWorld* World)
	{
		if (!World || !World->PersistentLevel)
		{
			return false;
		}

		const FString LevelPath = World->PersistentLevel->GetOutermost()->GetName();
		return LevelPath.Contains(TEXT("FrontendMap"));
	}

	bool IsServerRuntime(const UWorld* World)
	{
		return IsRunningDedicatedServer()
			|| FParse::Param(FCommandLine::Get(), TEXT("server"))
			|| (World && World->GetNetMode() == NM_DedicatedServer);
	}

	bool NameLooksLikeEnvironmentActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		const FString ActorName = Actor->GetName();
		static const TArray<FString> Keywords = {
			TEXT("Sky"),
			TEXT("Cloud"),
			TEXT("Atmosphere"),
			TEXT("Fog"),
			TEXT("Sun"),
			TEXT("Moon"),
			TEXT("Ocean"),
			TEXT("Sea"),
			TEXT("Water"),
			TEXT("Light"),
			TEXT("Lighting"),
			TEXT("Volumetric"),
			TEXT("Landscape"),
			TEXT("Terrain")
		};

		for (const FString& Keyword : Keywords)
		{
			if (ActorName.Contains(Keyword, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}

		return false;
	}

	void DisableFrontendViewportSceneRendering(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			FEngineShowFlags& ShowFlags = ViewportClient->EngineShowFlags;
			ShowFlags.SetAtmosphere(false);
			ShowFlags.SetFog(false);
			ShowFlags.SetVolumetricFog(false);
			// 保留 Lighting / DynamicShadows / SkyLighting，供角色选择/创建的世界 3D 展示舞台使用。
		}
	}

	void EnableFrontendCharacterPresentationRendering(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		if (UGameViewportClient* ViewportClient = World->GetGameViewport())
		{
			FEngineShowFlags& ShowFlags = ViewportClient->EngineShowFlags;
			ShowFlags.SetLighting(true);
			ShowFlags.SetDynamicShadows(true);
			ShowFlags.SetSkyLighting(true);
			ShowFlags.SetStaticMeshes(true);
			ShowFlags.SetSkeletalMeshes(true);
			ShowFlags.SetTranslucency(true);
			ShowFlags.SetPostProcessing(true);
			ShowFlags.SetTonemapper(true);
			// 选角/创建舞台自带雾效、SkyAtmosphere 与背景几何；体积云仍关闭以免盖过舞台。
			ShowFlags.SetFog(true);
			ShowFlags.SetVolumetricFog(true);
			ShowFlags.SetAtmosphere(true);
			ShowFlags.SetGame(true);
		}
	}

	bool IsCharacterPresentationFlowActive(const UWorld* World)
	{
		const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		const UDBAFrontendFlowSubsystem* LoginFlow = GameInstance ? GameInstance->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr;
		if (!LoginFlow)
		{
			return false;
		}

		const EDBALoginFlowState FlowState = LoginFlow->GetFlowState();
		return FlowState == EDBALoginFlowState::CharacterSelecting
			|| FlowState == EDBALoginFlowState::CharacterCreating;
	}

	int32 HideActorFromFrontendLogin(AActor* Actor)
	{
		if (!Actor || Actor->IsA<APlayerController>() || Actor->IsA<AGameModeBase>())
		{
			return 0;
		}

		// PostProcessVolume 在 Hidden 时仍会参与合成；必须显式关闭，否则登录/选角会被压成纯黑。
		if (APostProcessVolume* PostProcessVolume = Cast<APostProcessVolume>(Actor))
		{
			PostProcessVolume->bEnabled = false;
			PostProcessVolume->BlendWeight = 0.0f;
		}

		Actor->SetActorHiddenInGame(true);
		Actor->SetActorEnableCollision(false);
		Actor->SetActorTickEnabled(false);
		return 1;
	}

	int32 DisableConflictingPostProcessVolumes(UWorld* World)
	{
		if (!World)
		{
			return 0;
		}

		int32 DisabledCount = 0;
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			APostProcessVolume* PostProcessVolume = *It;
			if (!IsValid(PostProcessVolume))
			{
				continue;
			}

			if (PostProcessVolume->bEnabled || PostProcessVolume->BlendWeight > KINDA_SMALL_NUMBER)
			{
				PostProcessVolume->bEnabled = false;
				PostProcessVolume->BlendWeight = 0.0f;
				++DisabledCount;
			}

			PostProcessVolume->SetActorHiddenInGame(true);
			PostProcessVolume->SetActorEnableCollision(false);
			PostProcessVolume->SetActorTickEnabled(false);
		}
		return DisabledCount;
	}

	int32 CleanupFrontendEnvironmentActors(UWorld* World)
	{
		if (!World)
		{
			return 0;
		}

		int32 AffectedCount = 0;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!IsValid(Actor))
			{
				continue;
			}

			if (Actor->IsA<ADBACharacterPresentationActor>())
			{
				continue;
			}

			const bool bIsLandscapeActor = Actor->GetClass()->GetName().Contains(TEXT("Landscape"));
			const bool bIsEnvironmentClass =
				Actor->IsA<ADirectionalLight>()
				|| Actor->IsA<ASkyLight>()
				|| Actor->IsA<AExponentialHeightFog>()
				|| Actor->IsA<APostProcessVolume>()
				|| IsSkyAtmosphereActor(Actor)
				|| IsVolumetricCloudActor(Actor)
				|| IsWaterActor(Actor)
				|| bIsLandscapeActor
				|| Actor->IsA<AStaticMeshActor>();

			if (bIsEnvironmentClass
				&& (Actor->IsA<ADirectionalLight>()
					|| Actor->IsA<ASkyLight>()
					|| Actor->IsA<AExponentialHeightFog>()
					|| Actor->IsA<APostProcessVolume>()
					|| IsSkyAtmosphereActor(Actor)
					|| IsVolumetricCloudActor(Actor)
					|| IsWaterActor(Actor)
					|| bIsLandscapeActor
					|| NameLooksLikeEnvironmentActor(Actor)))
			{
				AffectedCount += HideActorFromFrontendLogin(Actor);
			}
		}

		return AffectedCount;
	}
}

bool UDBAFrontendEnvironmentSubsystem::IsSupportedInCurrentEnvironment() const
{
	return !DBAFrontendEnvironment::IsServerRuntime(GetWorld());
}

bool UDBAFrontendEnvironmentSubsystem::IsFrontendWorld() const
{
	return DBAFrontendEnvironment::IsFrontendWorld(GetWorld());
}

void UDBAFrontendEnvironmentSubsystem::OnWorldBeginPlayInternal()
{
	Super::OnWorldBeginPlayInternal();

	if (!IsFrontendWorld())
	{
		return;
	}

	ApplyFrontendUiOnlyEnvironment();
	ScheduleEnvironmentCleanupRetries();
}

void UDBAFrontendEnvironmentSubsystem::OnSubsystemDeinitialize()
{
	ClearEnvironmentCleanupTimers();
	Super::OnSubsystemDeinitialize();
}

void UDBAFrontendEnvironmentSubsystem::ApplyFrontendUiOnlyEnvironment()
{
	UWorld* World = GetWorld();
	if (!World || !IsFrontendWorld() || !IsSupportedInCurrentEnvironment())
	{
		return;
	}

	// 选角/创建阶段：只恢复展示渲染，不再重复隐藏关卡环境（舞台自带背景与光照）。
	if (DBAFrontendEnvironment::IsCharacterPresentationFlowActive(World))
	{
		EnableCharacterPresentationRendering();
		UE_LOG(LogDBAUI, Log, TEXT("[FrontendEnvironment] 角色展示流程激活，已保留/恢复舞台光照与雾效渲染。"));
		return;
	}

	DBAFrontendEnvironment::DisableFrontendViewportSceneRendering(World);
	const int32 HiddenCount = DBAFrontendEnvironment::CleanupFrontendEnvironmentActors(World);
	UE_LOG(LogDBAUI, Log, TEXT("[FrontendEnvironment] 已清理前端登录场景三维组件，隐藏 Actor 数量=%d"), HiddenCount);
}

void UDBAFrontendEnvironmentSubsystem::EnableCharacterPresentationRendering()
{
	UWorld* World = GetWorld();
	if (!World || !IsFrontendWorld() || !IsSupportedInCurrentEnvironment())
	{
		return;
	}

	DBAFrontendEnvironment::EnableFrontendCharacterPresentationRendering(World);
	const int32 DisabledPostProcessCount = DBAFrontendEnvironment::DisableConflictingPostProcessVolumes(World);
	UE_LOG(LogDBAUI, Log, TEXT("[FrontendEnvironment] 已恢复角色展示所需的光照与阴影渲染。已关闭关卡后处理体积数量=%d"), DisabledPostProcessCount);
}

void UDBAFrontendEnvironmentSubsystem::ScheduleEnvironmentCleanupRetries()
{
	UWorld* World = GetWorld();
	if (!World || !IsFrontendWorld())
	{
		return;
	}

	EnvironmentCleanupRetryCount = 0;
	ClearEnvironmentCleanupTimers();

	World->GetTimerManager().SetTimer(
		InitialCleanupTimerHandle,
		this,
		&UDBAFrontendEnvironmentSubsystem::ApplyFrontendUiOnlyEnvironment,
		0.1f,
		false);

	World->GetTimerManager().SetTimer(
		FollowUpCleanupTimerHandle,
		this,
		&UDBAFrontendEnvironmentSubsystem::HandleFollowUpEnvironmentCleanup,
		0.35f,
		false);
}

void UDBAFrontendEnvironmentSubsystem::HandleFollowUpEnvironmentCleanup()
{
	ApplyFrontendUiOnlyEnvironment();

	++EnvironmentCleanupRetryCount;
	if (EnvironmentCleanupRetryCount >= 6)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FollowUpCleanupTimerHandle,
			this,
			&UDBAFrontendEnvironmentSubsystem::HandleFollowUpEnvironmentCleanup,
			0.25f,
			false);
	}
}

void UDBAFrontendEnvironmentSubsystem::ClearEnvironmentCleanupTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InitialCleanupTimerHandle);
		World->GetTimerManager().ClearTimer(FollowUpCleanupTimerHandle);
	}
}
