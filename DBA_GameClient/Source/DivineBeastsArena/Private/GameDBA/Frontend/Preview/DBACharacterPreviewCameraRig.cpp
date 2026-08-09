// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Preview/DBACharacterPreviewCameraRig.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameDBA/Data/Assets/DBAZodiacHeroDataAsset.h"
#include "GameFramework/PlayerController.h"

ADBACharacterPreviewCameraRig::ADBACharacterPreviewCameraRig()
{
	bReplicates = false;
	SetCanBeDamaged(false);
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewCameraRoot"));
	SetRootComponent(Root);
	PreviewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PreviewCamera"));
	PreviewCamera->SetupAttachment(Root);
	PreviewCamera->bAutoActivate = false;
}

void ADBACharacterPreviewCameraRig::ApplyPreset(const FDBAZodiacPreviewCameraPreset& Preset)
{
	const FVector BaseLocation = InitialRelativeLocation.IsNearlyZero() ? PreviewCamera->GetRelativeLocation() : InitialRelativeLocation;
	const FRotator BaseRotation = InitialRelativeRotation.IsNearlyZero() ? PreviewCamera->GetRelativeRotation() : InitialRelativeRotation;
	if (InitialFieldOfView <= 0.0f)
	{
		InitialRelativeLocation = BaseLocation;
		InitialRelativeRotation = BaseRotation;
		InitialFieldOfView = PreviewCamera->FieldOfView;
	}

	ActiveMinDistance = Preset.MinDistance > 0.0f ? Preset.MinDistance : DefaultMinDistance;
	ActiveMaxDistance = Preset.MaxDistance > 0.0f ? Preset.MaxDistance : DefaultMaxDistance;
	ActiveDistance = Preset.Distance > 0.0f ? Preset.Distance : DefaultDistance;
	if (ActiveDistance <= 0.0f && !Preset.CameraOffset.IsNearlyZero())
	{
		ActiveDistance = Preset.CameraOffset.Size();
	}

	PreviewCamera->SetRelativeLocation(BaseLocation + Preset.CameraOffset + FVector(0.0f, 0.0f, Preset.Height));
	PreviewCamera->SetRelativeRotation(BaseRotation + Preset.CameraRotation);
	if (Preset.FieldOfView > 0.0f)
	{
		PreviewCamera->SetFieldOfView(Preset.FieldOfView);
	}
	else if (InitialFieldOfView > 0.0f)
	{
		PreviewCamera->SetFieldOfView(InitialFieldOfView);
	}
	ApplyDistance(ActiveDistance);
	ResetRelativeLocation = PreviewCamera->GetRelativeLocation();
	ResetRelativeRotation = PreviewCamera->GetRelativeRotation();
	ResetFieldOfView = PreviewCamera->FieldOfView;
	ResetDistance = ActiveDistance;
}

void ADBACharacterPreviewCameraRig::Zoom(const float DeltaDistance)
{
	if (ActiveDistance <= 0.0f || FMath::IsNearlyZero(DeltaDistance))
	{
		return;
	}
	ApplyDistance(ActiveDistance + DeltaDistance);
}

void ADBACharacterPreviewCameraRig::ResetCamera()
{
	if (InitialFieldOfView <= 0.0f)
	{
		return;
	}
	PreviewCamera->SetRelativeLocation(ResetRelativeLocation);
	PreviewCamera->SetRelativeRotation(ResetRelativeRotation);
	PreviewCamera->SetFieldOfView(ResetFieldOfView);
	ActiveDistance = ResetDistance;
}

void ADBACharacterPreviewCameraRig::Activate(APlayerController* PlayerController, const float BlendTime) const
{
	if (IsValid(PlayerController))
	{
		PlayerController->SetViewTargetWithBlend(const_cast<ADBACharacterPreviewCameraRig*>(this), BlendTime);
	}
}

void ADBACharacterPreviewCameraRig::ApplyDistance(float InDistance)
{
	if (InDistance <= 0.0f)
	{
		return;
	}
	if (ActiveMaxDistance > 0.0f && ActiveMinDistance > ActiveMaxDistance)
	{
		Swap(ActiveMinDistance, ActiveMaxDistance);
	}
	if (ActiveMinDistance > 0.0f)
	{
		InDistance = FMath::Max(InDistance, ActiveMinDistance);
	}
	if (ActiveMaxDistance > 0.0f)
	{
		InDistance = FMath::Min(InDistance, ActiveMaxDistance);
	}

	ActiveDistance = InDistance;
	const FVector Direction = PreviewCamera->GetRelativeLocation().GetSafeNormal();
	if (!Direction.IsNearlyZero())
	{
		PreviewCamera->SetRelativeLocation(Direction * ActiveDistance);
	}
}
