// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameFramework/Actor.h"
#include "DBACharacterPreviewActor.generated.h"

class USkeletalMeshComponent;
class UAnimationAsset;

UCLASS()
class DIVINEBEASTSARENA_API ADBACharacterPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ADBACharacterPreviewActor();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "DBA|Preview")
	void SetPreviewZodiac(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|Preview")
	void SetRotationSpeed(float InDegreesPerSecond);

protected:
	virtual void BeginPlay() override;

private:
	void ApplyPreviewAssets(EDBAZodiac Zodiac);
	static FString GetMeshPathByZodiac(EDBAZodiac Zodiac);
	static FString GetIdleAnimationPathByZodiac(EDBAZodiac Zodiac);
	static FString GetMaterialPathByZodiac(EDBAZodiac Zodiac);

private:
	UPROPERTY(VisibleAnywhere, Category = "DBA|Preview")
	TObjectPtr<USkeletalMeshComponent> PreviewMeshComponent;

	UPROPERTY(EditAnywhere, Category = "DBA|Preview")
	float RotationSpeedDegreesPerSecond = 0.0f;
};
