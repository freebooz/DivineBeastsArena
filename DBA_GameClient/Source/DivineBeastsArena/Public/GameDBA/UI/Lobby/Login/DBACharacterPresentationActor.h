// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameFramework/Actor.h"
#include "DBACharacterPresentationActor.generated.h"

class APlayerController;
class UAnimationAsset;
class UCameraComponent;
class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class UPostProcessComponent;
class UPointLightComponent;
class USceneComponent;
class USkeletalMeshComponent;
class USkyLightComponent;
class UStaticMeshComponent;
class USkeletalMesh;
class UMaterialInterface;
class UWorld;

USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBACharacterPresentationStageSpec
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	float CameraFOV = 34.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	FVector CameraLocation = FVector(520.0f, 0.0f, 150.0f);

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	FRotator CameraRotation = FRotator(-2.0f, 180.0f, 0.0f);

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	float KeyLightIntensity = 0.35f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	float FillLightIntensity = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	float RimLightIntensity = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	float FaceLightIntensity = 120.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	float SkyLightIntensity = 0.06f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	float PillarRedLightIntensity = 800.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	FVector GroundScale = FVector(7.5f, 7.5f, 1.0f);

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	bool bUseAtmosphericFog = true;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterPresentation")
	bool bUsePedestal = true;
};

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBACharacterPresentationActor : public AActor
{
	GENERATED_BODY()

public:
	ADBACharacterPresentationActor();

	UFUNCTION(BlueprintPure, Category = "DBA|CharacterPresentation")
	static FDBACharacterPresentationStageSpec GetReferenceStageSpec();

	static ADBACharacterPresentationActor* ResolveSharedPresentationStage(UWorld* World);
	static void ReleaseSharedPresentationStage(ADBACharacterPresentationActor*& InOutActor);
	static FString GetPreviewMeshPathForZodiac(EDBAZodiac Zodiac);
	static FString GetPreviewLegacyMeshPathForZodiac(EDBAZodiac Zodiac);
	static TArray<FString> GetLobbyDisplayMeshCandidatePathsForZodiac(EDBAZodiac Zodiac);
	static FString GetLobbyDisplayAnimBlueprintPathForMeshPath(const FString& MeshPath, EDBAZodiac Zodiac);
	static FString GetPreviewIdleAnimationPathForZodiac(EDBAZodiac Zodiac);
	static FString GetPreviewMaterialPathForZodiac(EDBAZodiac Zodiac);
	static FLinearColor GetPreviewTintForZodiac(EDBAZodiac Zodiac);
	static FRotator GetPreviewMeshPlayerFacingRotation();
	static bool ApplyLobbyDisplayAnimationToMesh(USkeletalMeshComponent* MeshComponent, const FString& MeshPath, EDBAZodiac Zodiac);
	static bool ApplyZodiacMaterialToMesh(USkeletalMeshComponent* MeshComponent, EDBAZodiac Zodiac, UObject* Outer);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterPresentation")
	void SetPreviewZodiac(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterPresentation")
	void AddPreviewYaw(float DeltaYawDegrees);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterPresentation")
	void ActivatePresentationCamera(APlayerController* PlayerController, float BlendTime = 0.18f);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void ApplyPreviewAssets(EDBAZodiac Zodiac);
	void ConfigureStageVisuals();
	void ApplyStageSpec();

private:
	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<USceneComponent> StageRoot;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<USkeletalMeshComponent> PreviewMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UCameraComponent> PresentationCamera;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UStaticMeshComponent> GroundPlane;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UStaticMeshComponent> Pedestal;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UStaticMeshComponent> BackdropPlane;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UStaticMeshComponent> LeftPillar;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UStaticMeshComponent> RightPillar;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UStaticMeshComponent> MoonDisc;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UDirectionalLightComponent> KeyLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UDirectionalLightComponent> FillLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UDirectionalLightComponent> RimLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UPointLightComponent> FaceLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UPointLightComponent> LeftPillarRedLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UPointLightComponent> RightPillarRedLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UExponentialHeightFogComponent> AtmosphereFog;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UPostProcessComponent> PostProcess;

	// Hard references to ensure zodiac preview assets are cooked into client builds.
	UPROPERTY()
	TArray<TObjectPtr<USkeletalMesh>> CookAnchorPreviewMeshes;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInterface>> CookAnchorPreviewMaterials;
};
