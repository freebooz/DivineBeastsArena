// Copyright Freebooz Games, Inc. All Rights Reserved.
// VFX数据结构 - 用于技能特效配置

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAVFXDataRow.generated.h"

class UParticleSystem;
class USoundBase;
class UAnimMontage;

/**
 * FDBAVFXDataRow
 * VFX数据行 - 通过ZodiacType + SkillSlot从DataTable加载
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAVFXDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** 生肖类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EDBAZodiac ZodiacType = EDBAZodiac::None;

	/** 技能槽位 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FName SkillSlot;

	// ==================== VFX资源 ====================

	/** 施法特效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> CastingVFX;

	/** 命中特效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> ImpactVFX;

	/** 飞行弹道特效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> ProjectileVFX;

	/** 范围爆炸特效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> AOEVFX;

	/** 引导持续特效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> ChannelVFX;

	/** 增益特效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> BuffVFX;

	/** 减益特效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> DebuffVFX;

	// ==================== SFX资源 ====================

	/** 施法音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> CastingSFX;

	/** 飞行弹道音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> ProjectileSFX;

	/** 命中音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> ImpactSFX;

	// ==================== 动画资源 ====================

	/** 施法动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> CastingMontage;

	/** 命中动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> ImpactMontage;
};