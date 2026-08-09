// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Frontend/Preview/DBAFiveCampPreviewTheme.h"
#include "GameFramework/Actor.h"
#include "DBACharacterPreviewStage.generated.h"

class ADBACharacterPreviewActor;
class ADBACharacterPreviewCameraRig;
class UAudioComponent;
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

	/**
	 * 将已解析的五营主题应用到纯前台舞台。此函数不触碰 PreviewActor 的 AppearanceComponent，
	 * 因而不会覆盖生肖、发型、颜色等已经应用的角色外观，也不会修改任何 TeamId。
	 */
	void ApplyFiveCampTheme(const FDBAFiveCampPreviewTheme& InTheme);

	/** 退出创建第三步或释放前台预览时清除主题音效与表现投影。 */
	void ClearFiveCampTheme();

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

	/** 仅播放五营选择主题音效；正式角色音频继续由 PreviewActor 自己管理。 */
	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview|FiveCamp")
	TObjectPtr<UAudioComponent> FiveCampThemeAudio;

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

	/** 当前主题仅为前台表现状态，蓝图可通过事件将已加载资源挂接到舞台锚点。 */
	UPROPERTY(Transient)
	FDBAFiveCampPreviewTheme CurrentFiveCampTheme;

	// BlueprintImplementableEvent 必须位于受保护或公开区；它们仍只服务于舞台美术组件，不对外暴露业务状态。
protected:
	/**
	 * 仅供 L_DBA_Frontend 中的表现蓝图绑定背景、徽记、VFX 材质等已有组件；
	 * 业务选择、资源解析、异步乱序保护全部由 C++ Controller/Subsystem 负责。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Preview|FiveCamp")
	void BP_OnFiveCampThemeApplied(const FDBAFiveCampPreviewTheme& InTheme);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Preview|FiveCamp")
	void BP_OnFiveCampThemeCleared();
};
