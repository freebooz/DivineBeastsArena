// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "DBAMobaGameplayEffectContext.generated.h"

/**
 * MOBA 游戏 GameplayEffectContext 扩展
 * 用于传递伤害计算所需的额外上下文信息：
 * - 自然元素之力克制系数
 * - 连锁等级
 * - 共鸣等级
 * - 暴击标记
 * - 伤害增加 / 减少临时系数
 *
 * 不扩展为长期同步属性，仅作为伤害计算时的临时上下文
 */
USTRUCT()
struct DIVINEBEASTSARENA_API FDBAMobaGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	FDBAMobaGameplayEffectContext()
		: FGameplayEffectContext()
		, ElementCounterFactor(1.0f)
		, ChainLevel(0)
		, ResonanceLevel(0)
		, bIsCriticalHit(false)
		, DamageIncreaseFactor(1.0f)
		, DamageReduceFactor(1.0f)
	{
	}

	/** 自然元素之力克制系数（1.25 克制 / 0.80 被克制 / 1.00 无关系） */
	UPROPERTY()
	float ElementCounterFactor;

	/** 连锁等级（0~10） */
	UPROPERTY()
	int32 ChainLevel;

	/** 共鸣等级（0~4） */
	UPROPERTY()
	int32 ResonanceLevel;

	/** 是否暴击 */
	UPROPERTY()
	bool bIsCriticalHit;

	/** 伤害增加系数（通用伤害增加） */
	UPROPERTY()
	float DamageIncreaseFactor;

	/** 伤害减少系数（通用伤害减少） */
	UPROPERTY()
	float DamageReduceFactor;

	// ========== FGameplayEffectContext 接口实现 ==========

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FDBAMobaGameplayEffectContext::StaticStruct();
	}

	virtual FDBAMobaGameplayEffectContext* Duplicate() const override
	{
		FDBAMobaGameplayEffectContext* NewContext = new FDBAMobaGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
};

template<>
struct TStructOpsTypeTraits<FDBAMobaGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FDBAMobaGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
