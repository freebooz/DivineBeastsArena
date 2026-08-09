// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Preview/DBACharacterPreviewSubsystem.h"

#include "GameDBA/Character/Data/DBAZodiacRegistrySubsystem.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Data/Assets/DBAZodiacHeroDataAsset.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterPresentationActor.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewActor.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewCameraRig.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewStage.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UDBACharacterPreviewSubsystem::Deinitialize()
{
	ReleasePreview();
	Super::Deinitialize();
}

bool UDBACharacterPreviewSubsystem::SelectZodiac(const EDBAZodiac Zodiac, const FDBACharacterAppearance& Appearance)
{
	if (Zodiac == EDBAZodiac::None || IsDedicatedServer())
	{
		return false;
	}
	const uint32 RequestGeneration = RequestGate.BeginRequest();
	if (ActiveZodiac != EDBAZodiac::None && ActiveZodiac != Zodiac)
	{
		if (UDBAZodiacRegistrySubsystem* Registry = GetGameInstance()->GetSubsystem<UDBAZodiacRegistrySubsystem>())
		{
			Registry->Release(ActiveZodiac);
		}
	}
	ActiveZodiac = Zodiac;

	if (UDBAZodiacRegistrySubsystem* Registry = GetGameInstance()->GetSubsystem<UDBAZodiacRegistrySubsystem>())
	{
		TWeakObjectPtr<UDBACharacterPreviewSubsystem> WeakThis(this);
		return Registry->LoadAsync(Zodiac, FDBAOnZodiacHeroAssetLoaded::CreateLambda([WeakThis, Appearance, RequestGeneration](const EDBAZodiac LoadedZodiac, UDBAZodiacHeroDataAsset* HeroData)
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnZodiacAssetLoaded(LoadedZodiac, HeroData, Appearance, RequestGeneration);
			}
		}));
	}

	UE_LOG(LogDBAPreview, Error, TEXT("[角色预览] ZodiacRegistrySubsystem 不可用，无法加载生肖预览。"));
	OnCharacterPreviewResolved.Broadcast(Zodiac, false);
	return false;
}

void UDBACharacterPreviewSubsystem::Rotate(const float DeltaYawDegrees)
{
	if (ActivePreviewActor.IsValid())
	{
		ActivePreviewActor->Rotate(DeltaYawDegrees);
		return;
	}
	if (ADBACharacterPresentationActor* LegacyStage = ResolveLegacyPresentationStage())
	{
		LegacyStage->AddPreviewYaw(DeltaYawDegrees);
	}
}

void UDBACharacterPreviewSubsystem::Zoom(const float DeltaDistance)
{
	if (ADBACharacterPreviewStage* Stage = ResolvePreviewStage())
	{
		if (ADBACharacterPreviewCameraRig* CameraRig = Stage->EnsureCameraRig())
		{
			CameraRig->Zoom(DeltaDistance);
		}
	}
}

void UDBACharacterPreviewSubsystem::ResetCamera()
{
	if (ADBACharacterPreviewStage* Stage = ResolvePreviewStage())
	{
		if (ADBACharacterPreviewCameraRig* CameraRig = Stage->EnsureCameraRig())
		{
			CameraRig->ResetCamera();
		}
	}
}

void UDBACharacterPreviewSubsystem::ActivateCamera(APlayerController* PlayerController, const float BlendTime)
{
	if (ADBACharacterPreviewStage* Stage = ResolvePreviewStage())
	{
		if (ADBACharacterPreviewCameraRig* CameraRig = Stage->EnsureCameraRig())
		{
			CameraRig->Activate(PlayerController, BlendTime);
			return;
		}
	}
	if (ADBACharacterPresentationActor* LegacyStage = ResolveLegacyPresentationStage())
	{
		LegacyStage->ActivatePresentationCamera(PlayerController, BlendTime);
	}
}

void UDBACharacterPreviewSubsystem::PlaySelect()
{
	if (ActivePreviewActor.IsValid())
	{
		ActivePreviewActor->PlaySelect();
	}
}

void UDBACharacterPreviewSubsystem::PlayIdleVariation()
{
	if (ActivePreviewActor.IsValid())
	{
		ActivePreviewActor->PlayIdleVariation();
	}
}

void UDBACharacterPreviewSubsystem::ReleasePreview()
{
	RequestGate.Invalidate();
	if (ActiveZodiac != EDBAZodiac::None && GetGameInstance())
	{
		if (UDBAZodiacRegistrySubsystem* Registry = GetGameInstance()->GetSubsystem<UDBAZodiacRegistrySubsystem>())
		{
			Registry->Release(ActiveZodiac);
		}
	}
	if (ADBACharacterPreviewStage* Stage = ResolvePreviewStage())
	{
		Stage->ReleasePreviewActor();
	}
	ActivePreviewActor.Reset();
	ActiveZodiac = EDBAZodiac::None;
}

bool UDBACharacterPreviewSubsystem::IsDedicatedServer() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return !GameInstance || !GameInstance->GetWorld() || GameInstance->GetWorld()->GetNetMode() == NM_DedicatedServer;
}

ADBACharacterPreviewStage* UDBACharacterPreviewSubsystem::ResolvePreviewStage() const
{
	return GetGameInstance() ? ADBACharacterPreviewStage::FindPlacedPreviewStage(GetGameInstance()->GetWorld()) : nullptr;
}

ADBACharacterPresentationActor* UDBACharacterPreviewSubsystem::ResolveLegacyPresentationStage() const
{
	return GetGameInstance() ? ADBACharacterPresentationActor::FindPlacedPresentationStage(GetGameInstance()->GetWorld()) : nullptr;
}

void UDBACharacterPreviewSubsystem::OnZodiacAssetLoaded(const EDBAZodiac Zodiac, UDBAZodiacHeroDataAsset* HeroData, const FDBACharacterAppearance Appearance, const uint32 RequestGeneration)
{
	if (!RequestGate.IsCurrent(RequestGeneration) || Zodiac != ActiveZodiac)
	{
		return;
	}
	ADBACharacterPreviewStage* Stage = ResolvePreviewStage();
	if (!Stage)
	{
		// 旧 FrontendMap 尚未放置新 Stage 时，只作为过渡回退；不生成 Gameplay Character。
		if (ADBACharacterPresentationActor* LegacyStage = ResolveLegacyPresentationStage())
		{
			LegacyStage->SetPreviewZodiac(Zodiac);
			LegacyStage->ApplyPreviewAppearance(Zodiac, Appearance);
			OnCharacterPreviewResolved.Broadcast(Zodiac, true);
			return;
		}
		UE_LOG(LogDBAPreview, Warning, TEXT("[角色预览] 当前前台地图未放置新 PreviewStage，且不存在可用的旧展示舞台。"));
		OnCharacterPreviewResolved.Broadcast(Zodiac, false);
		return;
	}
	if (!HeroData)
	{
		UE_LOG(LogDBAPreview, Error, TEXT("[角色预览] 生肖 %d 的静态配置加载失败。"), static_cast<int32>(Zodiac));
		OnCharacterPreviewResolved.Broadcast(Zodiac, false);
		return;
	}

	ADBACharacterPreviewActor* PreviewActor = Stage->EnsurePreviewActor();
	ADBACharacterPreviewCameraRig* CameraRig = Stage->EnsureCameraRig();
	if (!PreviewActor || !CameraRig)
	{
		OnCharacterPreviewResolved.Broadcast(Zodiac, false);
		return;
	}
	CameraRig->ApplyPreset(HeroData->CameraPreset);
	CameraRig->Activate(GetGameInstance()->GetFirstLocalPlayerController(), 0.0f);
	ActivePreviewActor = PreviewActor;
	const bool bStarted = PreviewActor->ApplyZodiacData(*HeroData, Appearance, RequestGeneration);
	OnCharacterPreviewResolved.Broadcast(Zodiac, bStarted);
}
