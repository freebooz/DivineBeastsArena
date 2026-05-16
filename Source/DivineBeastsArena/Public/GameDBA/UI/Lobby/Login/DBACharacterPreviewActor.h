// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameFramework/Actor.h"
#include "DBACharacterPreviewActor.generated.h"

class USkeletalMeshComponent;
class UAnimationAsset;
class UPointLightComponent;
class USceneComponent;

UCLASS()
class DIVINEBEASTSARENA_API ADBACharacterPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ADBACharacterPreviewActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "DBA|Preview")
	void SetPreviewZodiac(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|Preview")
	void SetRotationSpeed(float InDegreesPerSecond);

	EDBAZodiac GetPreviewZodiac() const { return CurrentZodiac; }

protected:
	virtual void BeginPlay() override;

private:
	void ApplyPreviewAssets(EDBAZodiac Zodiac);

	UFUNCTION()
	void OnRep_CurrentZodiac();

private:
	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USceneComponent> PreviewRoot;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USkeletalMeshComponent> PreviewMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<UPointLightComponent> ZodiacTintLight;

	UPROPERTY(EditAnywhere, Category = "DBA|Preview")
	float RotationSpeedDegreesPerSecond = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentZodiac)
	EDBAZodiac CurrentZodiac = EDBAZodiac::Rat;
};
