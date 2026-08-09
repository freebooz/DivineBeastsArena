// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBACharacterPreviewStage.generated.h"

class ADBACharacterPreviewActor;
class ADBACharacterPreviewCameraRig;
class UDirectionalLightComponent;
class USceneComponent;
class UWorld;

/**
 * 放置在持久 Frontend 地图的纯展示舞台：提供生成点、镜头锚点、三点光和背景/VFX 锚点。
 * 不持有角色业务状态，也不在 Dedicated Server 上创建预览对象。
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API ADBACharacterPreviewStage final : public AActor
{
	GENERATED_BODY()

public:
	ADBACharacterPreviewStage();

	static ADBACharacterPreviewStage* FindPlacedPreviewStage(UWorld* World);
	ADBACharacterPreviewActor* EnsurePreviewActor();
	ADBACharacterPreviewCameraRig* EnsureCameraRig();
	void ReleasePreviewActor();

	USceneComponent* GetSpawnPoint() const { return SpawnPoint; }
	USceneComponent* GetBackgroundAnchor() const { return BackgroundAnchor; }
	USceneComponent* GetVfxAnchor() const { return VfxAnchor; }

private:
	bool IsDedicatedServer() const;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USceneComponent> SpawnPoint;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USceneComponent> CameraRigAnchor;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USceneComponent> BackgroundAnchor;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USceneComponent> VfxAnchor;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview|Lighting")
	TObjectPtr<UDirectionalLightComponent> KeyLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview|Lighting")
	TObjectPtr<UDirectionalLightComponent> FillLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview|Lighting")
	TObjectPtr<UDirectionalLightComponent> RimLight;

	UPROPERTY(EditAnywhere, Category = "DBA|Preview")
	TSubclassOf<ADBACharacterPreviewActor> PreviewActorClass;

	UPROPERTY(EditAnywhere, Category = "DBA|Preview")
	TSubclassOf<ADBACharacterPreviewCameraRig> CameraRigClass;

	UPROPERTY(Transient)
	TObjectPtr<ADBACharacterPreviewActor> PreviewActor;

	UPROPERTY(Transient)
	TObjectPtr<ADBACharacterPreviewCameraRig> CameraRig;
};
