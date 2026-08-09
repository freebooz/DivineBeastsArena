// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBACharacterPreviewCameraRig.generated.h"

struct FDBAZodiacPreviewCameraPreset;
class APlayerController;
class UCameraComponent;
class USceneComponent;

/**
 * 前台角色预览专用摄像机。相机参数优先读取生肖 DataAsset；本 Actor 的可编辑默认值仅为地图未覆盖时的场景配置。
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API ADBACharacterPreviewCameraRig final : public AActor
{
	GENERATED_BODY()

public:
	ADBACharacterPreviewCameraRig();

	void ApplyPreset(const FDBAZodiacPreviewCameraPreset& Preset);
	void Zoom(float DeltaDistance);
	void ResetCamera();
	void Activate(APlayerController* PlayerController, float BlendTime = 0.0f) const;

	UCameraComponent* GetCameraComponent() const { return PreviewCamera; }

private:
	void ApplyDistance(float InDistance);

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<UCameraComponent> PreviewCamera;

	UPROPERTY(EditAnywhere, Category = "DBA|Preview|Camera", meta = (ClampMin = "0.0"))
	float DefaultDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "DBA|Preview|Camera", meta = (ClampMin = "0.0"))
	float DefaultMinDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "DBA|Preview|Camera", meta = (ClampMin = "0.0"))
	float DefaultMaxDistance = 0.0f;

	float ActiveDistance = 0.0f;
	float ActiveMinDistance = 0.0f;
	float ActiveMaxDistance = 0.0f;
	FVector InitialRelativeLocation = FVector::ZeroVector;
	FRotator InitialRelativeRotation = FRotator::ZeroRotator;
	float InitialFieldOfView = 0.0f;
	FVector ResetRelativeLocation = FVector::ZeroVector;
	FRotator ResetRelativeRotation = FRotator::ZeroRotator;
	float ResetFieldOfView = 0.0f;
	float ResetDistance = 0.0f;
};
