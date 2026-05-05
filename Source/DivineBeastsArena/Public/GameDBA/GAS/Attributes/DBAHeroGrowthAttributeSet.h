// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DBAHeroGrowthAttributeSet.generated.h"

// 成长属性宏：简化 AttributeAccessor 定义
#define HERO_GROWTH_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 英雄成长属性集
 *
 * 包含英雄专属的成长和状态属性：
 * - HeroLevel：英雄等级
 * - Experience：当前经验值
 * - RespawnTime：复活时间（秒）
 * - GoldBounty：击杀奖励金币
 *
 * 设计原则：
 * - 仅英雄单位拥有此属性集
 * - 成长属性通过经验结算系统修改
 * - 不影响核心战斗公式
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAHeroGrowthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UDBAHeroGrowthAttributeSet();

	//~ Begin UAttributeSet Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UAttributeSet Interface

	// ========================================
	// 等级与经验
	// ========================================

	/** 英雄等级 */
	UPROPERTY(BlueprintReadOnly, Category = "Hero|Growth", ReplicatedUsing = OnRep_HeroLevel)
	FGameplayAttributeData HeroLevel;
	HERO_GROWTH_ATTRIBUTE_ACCESSORS(UDBAHeroGrowthAttributeSet, HeroLevel)

	UFUNCTION()
	virtual void OnRep_HeroLevel(const FGameplayAttributeData& OldHeroLevel);

	/** 当前经验值 */
	UPROPERTY(BlueprintReadOnly, Category = "Hero|Growth", ReplicatedUsing = OnRep_Experience)
	FGameplayAttributeData Experience;
	HERO_GROWTH_ATTRIBUTE_ACCESSORS(UDBAHeroGrowthAttributeSet, Experience)

	UFUNCTION()
	virtual void OnRep_Experience(const FGameplayAttributeData& OldExperience);

	/** 下一级所需经验 */
	UPROPERTY(BlueprintReadOnly, Category = "Hero|Growth", ReplicatedUsing = OnRep_ExperienceToNextLevel)
	FGameplayAttributeData ExperienceToNextLevel;
	HERO_GROWTH_ATTRIBUTE_ACCESSORS(UDBAHeroGrowthAttributeSet, ExperienceToNextLevel)

	UFUNCTION()
	virtual void OnRep_ExperienceToNextLevel(const FGameplayAttributeData& OldExperienceToNextLevel);

	// ========================================
	// 复活与金币
	// ========================================

	/** 复活时间（秒） */
	UPROPERTY(BlueprintReadOnly, Category = "Hero|State", ReplicatedUsing = OnRep_RespawnTime)
	FGameplayAttributeData RespawnTime;
	HERO_GROWTH_ATTRIBUTE_ACCESSORS(UDBAHeroGrowthAttributeSet, RespawnTime)

	UFUNCTION()
	virtual void OnRep_RespawnTime(const FGameplayAttributeData& OldRespawnTime);

	/** 击杀奖励金币 */
	UPROPERTY(BlueprintReadOnly, Category = "Hero|State", ReplicatedUsing = OnRep_GoldBounty)
	FGameplayAttributeData GoldBounty;
	HERO_GROWTH_ATTRIBUTE_ACCESSORS(UDBAHeroGrowthAttributeSet, GoldBounty)

	UFUNCTION()
	virtual void OnRep_GoldBounty(const FGameplayAttributeData& OldGoldBounty);
};
