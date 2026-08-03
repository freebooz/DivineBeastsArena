// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 文件职责：集中配置投射物与法术的 VFX/SFX 软引用。
- 约束：战斗 C++ 只读取此配置，不直接写入资源路径；资源加载继续由现有异步预加载流程完成。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DBAProjectilePresentationDeveloperSettings.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA 投射物表现配置"))
class DIVINEBEASTSARENA_API UDBAProjectilePresentationDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ChainLightning")
	TSoftObjectPtr<UNiagaraSystem> ChainLightningArcVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ChainLightning")
	TSoftObjectPtr<UNiagaraSystem> ChainLightningBranchVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ChainLightning")
	TSoftObjectPtr<UNiagaraSystem> ChainLightningImpactVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ChainLightning")
	TSoftObjectPtr<USoundBase> ChainLightningCastSFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ChainLightning")
	TSoftObjectPtr<USoundBase> ChainLightningFlightSFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ChainLightning")
	TSoftObjectPtr<USoundBase> ChainLightningImpactSFX;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|HolyShield")
	TSoftObjectPtr<UNiagaraSystem> HolyShieldCastVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|HolyShield")
	TSoftObjectPtr<UNiagaraSystem> HolyShieldBarrierVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|HolyShield")
	TSoftObjectPtr<UNiagaraSystem> HolyShieldFlightVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|HolyShield")
	TSoftObjectPtr<UNiagaraSystem> HolyShieldImpactVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|HolyShield")
	TSoftObjectPtr<USoundBase> HolyShieldCastSFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|HolyShield")
	TSoftObjectPtr<USoundBase> HolyShieldSustainSFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|HolyShield")
	TSoftObjectPtr<USoundBase> HolyShieldImpactSFX;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|FrostShard")
	TSoftObjectPtr<UNiagaraSystem> FrostShardProjectileVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|FrostShard")
	TSoftObjectPtr<UNiagaraSystem> FrostShardCrystalWakeVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|FrostShard")
	TSoftObjectPtr<UNiagaraSystem> FrostShardMistWakeVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|FrostShard")
	TSoftObjectPtr<UNiagaraSystem> FrostShardSpiralWakeVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|FrostShard")
	TSoftObjectPtr<UNiagaraSystem> FrostShardImpactVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|FrostShard")
	TSoftObjectPtr<UNiagaraSystem> FrostShardSecondaryImpactVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|FrostShard")
	TSoftObjectPtr<UNiagaraSystem> FrostShardRingImpactVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|FrostShard")
	TSoftObjectPtr<USoundBase> FrostShardFlightSFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|FrostShard")
	TSoftObjectPtr<USoundBase> FrostShardImpactSFX;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ShadowBolt")
	TSoftObjectPtr<UNiagaraSystem> ShadowBoltProjectileVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ShadowBolt")
	TSoftObjectPtr<UNiagaraSystem> ShadowBoltWakeVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ShadowBolt")
	TSoftObjectPtr<UNiagaraSystem> ShadowBoltImpactVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ShadowBolt")
	TSoftObjectPtr<UNiagaraSystem> ShadowBoltSecondaryImpactVFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ShadowBolt")
	TSoftObjectPtr<USoundBase> ShadowBoltFlightSFX;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|ShadowBolt")
	TSoftObjectPtr<USoundBase> ShadowBoltImpactSFX;
};
