// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "DBAMobaAbilityGrantTypes.generated.h"

class UDBAMobaGameplayAbilityBase;
class UGameplayEffect;
class UAttributeSet;

/**
 * Ability 赋予句柄
 * 用于追踪已赋予的 Ability，支持后续移除
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMobaAbilityGrantHandle
{
	GENERATED_BODY()

	/** Ability Spec Handle */
	UPROPERTY()
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	/** 是否有效 */
	bool IsValid() const { return AbilitySpecHandle.IsValid(); }

	/** 重置句柄 */
	void Reset() { AbilitySpecHandle = FGameplayAbilitySpecHandle(); }
};

/**
 * GameplayEffect 赋予句柄
 * 用于追踪已赋予的 GameplayEffect，支持后续移除
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMobaEffectGrantHandle
{
	GENERATED_BODY()

	/** Active GameplayEffect Handle */
	UPROPERTY()
	FActiveGameplayEffectHandle EffectHandle;

	/** 是否有效 */
	bool IsValid() const { return EffectHandle.IsValid(); }

	/** 重置句柄 */
	void Reset() { EffectHandle = FActiveGameplayEffectHandle(); }
};

/**
 * AttributeSet 赋予句柄
 * 用于追踪已赋予的 AttributeSet，支持后续移除
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMobaAttributeSetGrantHandle
{
	GENERATED_BODY()

	/** AttributeSet 实例 */
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	/** 是否有效 */
	bool IsValid() const { return AttributeSet != nullptr; }

	/** 重置句柄 */
	void Reset() { AttributeSet = nullptr; }
};

/**
 * Ability 赋予配置
 * 定义单个 Ability 的赋予参数
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMobaAbilityGrantConfig
{
	GENERATED_BODY()

	/** Ability 类 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Grant", meta = (DisplayName = "Ability 类"))
	TSoftClassPtr<UDBAMobaGameplayAbilityBase> AbilityClass;

	/** Ability 等级 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Grant", meta = (DisplayName = "Ability 等级"))
	int32 AbilityLevel = 1;

	/** 输入 ID（用于绑定输入） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Grant", meta = (DisplayName = "输入 ID"))
	int32 InputID = -1;
};

/**
 * GameplayEffect 赋予配置
 * 定义单个 GameplayEffect 的赋予参数
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMobaEffectGrantConfig
{
	GENERATED_BODY()

	/** GameplayEffect 类 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect Grant", meta = (DisplayName = "GameplayEffect 类"))
	TSoftClassPtr<UGameplayEffect> EffectClass;

	/** Effect 等级 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect Grant", meta = (DisplayName = "Effect 等级"))
	float EffectLevel = 1.0f;
};

/**
 * AttributeSet 赋予配置
 * 定义单个 AttributeSet 的赋予参数
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMobaAttributeSetGrantConfig
{
	GENERATED_BODY()

	/** AttributeSet 类 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AttributeSet Grant", meta = (DisplayName = "AttributeSet 类"))
	TSoftClassPtr<UAttributeSet> AttributeSetClass;
};

/**
 * Ability Set 赋予结果
 * 记录一次 Ability Set 赋予操作的所有句柄
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMobaAbilitySetGrantResult
{
	GENERATED_BODY()

	/** 已赋予的 Ability 句柄列表 */
	UPROPERTY()
	TArray<FDBAMobaAbilityGrantHandle> GrantedAbilities;

	/** 已赋予的 GameplayEffect 句柄列表 */
	UPROPERTY()
	TArray<FDBAMobaEffectGrantHandle> GrantedEffects;

	/** 已赋予的 AttributeSet 句柄列表 */
	UPROPERTY()
	TArray<FDBAMobaAttributeSetGrantHandle> GrantedAttributeSets;

	/** 是否有效（至少有一个句柄有效） */
	bool IsValid() const
	{
		return GrantedAbilities.Num() > 0 || GrantedEffects.Num() > 0 || GrantedAttributeSets.Num() > 0;
	}

	/** 重置所有句柄 */
	void Reset()
	{
		GrantedAbilities.Reset();
		GrantedEffects.Reset();
		GrantedAttributeSets.Reset();
	}
};
