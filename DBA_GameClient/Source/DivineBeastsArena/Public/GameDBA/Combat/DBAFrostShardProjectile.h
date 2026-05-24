// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明原创冰霜投射物，用于表现蓝白冰核、寒雾拖尾与冰裂命中反馈。
- 修改提示：该类只配置项目内已有 Niagara/SFX 资源，不复刻第三方游戏的具体特效资产或镜头节奏。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Combat/DBASkillProjectileBase.h"
#include "DBAFrostShardProjectile.generated.h"

class UAudioComponent;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;
class UPointLightComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAFrostShardProjectile : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAFrostShardProjectile();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnProjectileHit(AActor* HitActor, FVector HitLocation) override;
	virtual void PreloadPresentationAssets() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FrostCore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FrostShardA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FrostShardB;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FrostShardC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FrostShardD;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> FrostCrystalWake;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> FrostMistWake;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> FrostSpiralWake;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> FrostLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> FrostTipLight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Frost")
	TObjectPtr<UMaterialInterface> FrostCoreMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Frost")
	TSoftObjectPtr<UNiagaraSystem> CrystalWakeVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Frost")
	TSoftObjectPtr<UNiagaraSystem> MistWakeVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Frost")
	TSoftObjectPtr<UNiagaraSystem> SpiralWakeVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Frost")
	TSoftObjectPtr<UNiagaraSystem> SecondaryImpactVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Frost")
	TSoftObjectPtr<UNiagaraSystem> RingImpactVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Frost")
	FLinearColor FrostColor = FLinearColor(0.42f, 0.86f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Frost", meta = (ClampMin = "0.0"))
	float FrostPulseSpeed = 11.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Frost", meta = (ClampMin = "0.0"))
	float FrostPulseAmount = 0.16f;

private:
	void ApplyFrostMaterial(UStaticMeshComponent* Mesh, float EmissiveStrength, float Alpha) const;
	void ActivateNiagaraComponent(UNiagaraComponent* Component, const TSoftObjectPtr<UNiagaraSystem>& Asset) const;
	void DeactivateLayeredVFX();
	void SpawnImpactLayer(const TSoftObjectPtr<UNiagaraSystem>& Asset, const FVector& Location, const FRotator& Rotation, const FVector& Scale) const;

	float AgeSeconds = 0.0f;
};
