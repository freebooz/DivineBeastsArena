// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameFramework/Actor.h"
#include "DBACharacterPresentationActor.generated.h"

class APlayerController;
class UAnimationAsset;
class UCameraComponent;
class UDirectionalLightComponent;
class UPointLightComponent;
class USceneComponent;
class USkeletalMeshComponent;
class USkyLightComponent;
class UStaticMeshComponent;

UCLASS()
class DIVINEBEASTSARENA_API ADBACharacterPresentationActor : public AActor
{
	GENERATED_BODY()

public:
	ADBACharacterPresentationActor();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterPresentation")
	void SetPreviewZodiac(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterPresentation")
	void AddPreviewYaw(float DeltaYawDegrees);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterPresentation")
	void ActivatePresentationCamera(APlayerController* PlayerController, float BlendTime = 0.18f);

protected:
	virtual void BeginPlay() override;

private:
	void ApplyPreviewAssets(EDBAZodiac Zodiac);
	void ConfigureStageVisuals();

	static FString GetMeshPathByZodiac(EDBAZodiac Zodiac);
	static FString GetIdleAnimationPathByZodiac(EDBAZodiac Zodiac);
	static FString GetMaterialPathByZodiac(EDBAZodiac Zodiac);

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
	TObjectPtr<UStaticMeshComponent> BackdropPlane;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UDirectionalLightComponent> KeyLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UDirectionalLightComponent> FillLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UDirectionalLightComponent> RimLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<UPointLightComponent> FaceLight;

	UPROPERTY(VisibleAnywhere, Category = "DBA|CharacterPresentation")
	TObjectPtr<USkyLightComponent> SkyLight;
};
