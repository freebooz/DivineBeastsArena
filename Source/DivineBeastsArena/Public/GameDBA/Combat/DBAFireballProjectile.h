// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Combat/DBASkillProjectileBase.h"
#include "DBAFireballProjectile.generated.h"

class UMaterialInterface;
class UAudioComponent;
class UPointLightComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAFireballProjectile : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAFireballProjectile();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnProjectileHit(AActor* HitActor, FVector HitLocation) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FireballCore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> FireballLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> FireballLoopAudio;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Fireball")
	TObjectPtr<UMaterialInterface> FireballCoreMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Fireball")
	FLinearColor FireballColor = FLinearColor(1.0f, 0.22f, 0.02f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Fireball")
	float FireballPulseSpeed = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Fireball")
	float FireballPulseAmount = 0.12f;

private:
	float AgeSeconds = 0.0f;
};
