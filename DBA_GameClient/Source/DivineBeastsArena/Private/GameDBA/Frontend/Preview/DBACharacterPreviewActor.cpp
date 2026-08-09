// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Preview/DBACharacterPreviewActor.h"

#include "Animation/AnimInstance.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StreamableManager.h"
#include "GameDBA/Character/Appearance/DBACharacterAppearanceComponent.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Data/Assets/DBAZodiacHeroDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

ADBACharacterPreviewActor::ADBACharacterPreviewActor()
{
	bReplicates = false;
	SetCanBeDamaged(false);
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewRoot"));
	SetRootComponent(Root);
	PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(Root);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMesh->SetGenerateOverlapEvents(false);
	AppearanceComponent = CreateDefaultSubobject<UDBACharacterAppearanceComponent>(TEXT("AppearanceComponent"));
	AppearanceComponent->SetBaseMeshComponent(PreviewMesh);
	PreviewVfx = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PreviewVfx"));
	PreviewVfx->SetupAttachment(Root);
	PreviewVfx->SetAutoActivate(false);
	PreviewAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("PreviewAudio"));
	PreviewAudio->SetupAttachment(Root);
	PreviewAudio->bAutoActivate = false;
}

bool ADBACharacterPreviewActor::ApplyZodiacData(const UDBAZodiacHeroDataAsset& HeroData, const FDBACharacterAppearance& Appearance, const uint32 RequestGeneration)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		UE_LOG(LogDBAPreview, Warning, TEXT("[角色预览] Dedicated Server 拒绝创建或加载前台角色预览资源。"));
		return false;
	}

	ActiveRequestGeneration = RequestGeneration;
	ActiveHeroData = &HeroData;
	if (ActiveVisualLoadHandle.IsValid())
	{
		ActiveVisualLoadHandle->CancelHandle();
		ActiveVisualLoadHandle.Reset();
	}

	TArray<FSoftObjectPath> Paths;
	if (!HeroData.BodyMesh.IsNull()) { Paths.Add(HeroData.BodyMesh.ToSoftObjectPath()); }
	if (!HeroData.AnimationBlueprintClass.IsNull()) { Paths.Add(HeroData.AnimationBlueprintClass.ToSoftObjectPath()); }
	if (!HeroData.IdleAnimation.IsNull()) { Paths.Add(HeroData.IdleAnimation.ToSoftObjectPath()); }
	if (!HeroData.SelectAnimation.IsNull()) { Paths.Add(HeroData.SelectAnimation.ToSoftObjectPath()); }
	if (!HeroData.PreviewVFX.IsNull()) { Paths.Add(HeroData.PreviewVFX.ToSoftObjectPath()); }
	if (!HeroData.PreviewSFX.IsNull()) { Paths.Add(HeroData.PreviewSFX.ToSoftObjectPath()); }

	if (Paths.IsEmpty())
	{
		UE_LOG(LogDBAPreview, Warning, TEXT("[角色预览] 生肖 %d 缺少可加载的预览资源。"), static_cast<int32>(HeroData.ZodiacType));
		return false;
	}

	TWeakObjectPtr<ADBACharacterPreviewActor> WeakThis(this);
	const TWeakObjectPtr<const UDBAZodiacHeroDataAsset> WeakHeroData(&HeroData);
	ActiveVisualLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths, FStreamableDelegate::CreateLambda([WeakThis, WeakHeroData, Appearance, RequestGeneration]()
	{
		if (WeakThis.IsValid() && WeakHeroData.IsValid())
		{
			WeakThis->CompleteVisualLoad(RequestGeneration, WeakHeroData.Get(), Appearance);
		}
	}));
	return ActiveVisualLoadHandle.IsValid();
}

void ADBACharacterPreviewActor::Rotate(const float DeltaYawDegrees)
{
	if (!FMath::IsNearlyZero(DeltaYawDegrees))
	{
		AddActorLocalRotation(FRotator(0.0f, DeltaYawDegrees, 0.0f));
	}
}

void ADBACharacterPreviewActor::PlaySelect()
{
	const UDBAZodiacHeroDataAsset* HeroData = ActiveHeroData.Get();
	if (!HeroData)
	{
		return;
	}
	if (UAnimationAsset* SelectAnimation = HeroData->SelectAnimation.Get())
	{
		PreviewMesh->PlayAnimation(SelectAnimation, false);
	}
	if (HeroData->PreviewVFX.Get())
	{
		PreviewVfx->SetAsset(HeroData->PreviewVFX.Get());
		PreviewVfx->Activate(true);
	}
	if (USoundBase* PreviewSound = HeroData->PreviewSFX.Get())
	{
		PreviewAudio->SetSound(PreviewSound);
		PreviewAudio->Play();
	}
}

void ADBACharacterPreviewActor::PlayIdleVariation()
{
	if (const UDBAZodiacHeroDataAsset* HeroData = ActiveHeroData.Get())
	{
		if (UAnimationAsset* IdleAnimation = HeroData->IdleAnimation.Get())
		{
			PreviewMesh->PlayAnimation(IdleAnimation, true);
		}
	}
}

void ADBACharacterPreviewActor::ReleasePreviewResources()
{
	++ActiveRequestGeneration;
	if (ActiveVisualLoadHandle.IsValid())
	{
		ActiveVisualLoadHandle->CancelHandle();
		ActiveVisualLoadHandle.Reset();
	}
	PreviewAudio->Stop();
	PreviewAudio->SetSound(nullptr);
	PreviewVfx->DeactivateImmediate();
	PreviewVfx->SetAsset(nullptr);
	AppearanceComponent->Reset();
	PreviewMesh->SetSkeletalMesh(nullptr);
	ActiveHeroData.Reset();
}

void ADBACharacterPreviewActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleasePreviewResources();
	Super::EndPlay(EndPlayReason);
}

void ADBACharacterPreviewActor::CompleteVisualLoad(const uint32 RequestGeneration, const UDBAZodiacHeroDataAsset* HeroData, const FDBACharacterAppearance Appearance)
{
	if (!HeroData || !IsCurrentRequest(RequestGeneration))
	{
		return;
	}
	ActiveVisualLoadHandle.Reset();
	USkeletalMesh* BodyMesh = HeroData->BodyMesh.Get();
	if (!BodyMesh)
	{
		UE_LOG(LogDBAPreview, Error, TEXT("[角色预览] 生肖 %d 的预览骨骼网格加载失败。"), static_cast<int32>(HeroData->ZodiacType));
		return;
	}

	PreviewMesh->SetSkeletalMesh(BodyMesh);
	AppearanceComponent->SetBaseMeshComponent(PreviewMesh);
	if (UClass* AnimationClass = HeroData->AnimationBlueprintClass.Get())
	{
		PreviewMesh->SetAnimInstanceClass(AnimationClass);
	}
	else
	{
		PlayIdleVariation();
	}
	AppearanceComponent->ApplyAppearance(HeroData->ZodiacType, Appearance);
	UE_LOG(LogDBAPreview, Log, TEXT("[角色预览] 已异步应用生肖 %d 的展示资源，请求代次=%u。"), static_cast<int32>(HeroData->ZodiacType), RequestGeneration);
}

bool ADBACharacterPreviewActor::IsCurrentRequest(const uint32 RequestGeneration) const
{
	return RequestGeneration != 0 && RequestGeneration == ActiveRequestGeneration;
}
