// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DBABattleAttributeSet.generated.h"

// 战斗属性宏：简化 AttributeAccessor 定义
#define BATTLE_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 战斗属性集
 *
 * 包含所有角色共用的战斗属性：
 * - MaxHealth / CurrentHealth：生命值
 * - AttackPower：攻击力
 * - Defense：防御力（物理防御）
 * - MoveSpeed：移动速度
 * - MaxEnergy / CurrentEnergy：能量（技能消耗）
 * - EnergyRegen：能量回复速度
 * - CriticalRate：暴击率（0~1）
 * - CriticalMultiplier：暴击倍率
 *
 * 设计原则：
 * - 所有战斗单位（英雄、小兵、防御塔）共享此属性集
 * - 属性修改通过 GameplayEffect 的 Modifier 实现
 * - 不在此属性集中定义成长属性（经验、金币等）
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBABattleAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UDBABattleAttributeSet();

	//~ Begin UAttributeSet Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	//~ End UAttributeSet Interface

	// ========== 辅助计算函数 ==========

	/**
	 * 计算物理伤害减免系数
	 * DamageReduction = Defense / (Defense + 100)
	 */
	UFUNCTION(BlueprintCallable, Category = "Battle|Combat")
	float CalculatePhysicalDamageReduction() const;

	/**
	 * 判定是否触发暴击
	 */
	UFUNCTION(BlueprintCallable, Category = "Battle|Combat")
	bool RollCriticalHit() const;

	// ========================================
	// 生命值
	// ========================================

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, MaxHealth)

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Health", ReplicatedUsing = OnRep_CurrentHealth)
	FGameplayAttributeData CurrentHealth;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, CurrentHealth)

	UFUNCTION()
	virtual void OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth);

	// ========================================
	// 攻击属性
	// ========================================

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Attack", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, AttackPower)

	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Attack", ReplicatedUsing = OnRep_Defense)
	FGameplayAttributeData Defense;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, Defense)

	UFUNCTION()
	virtual void OnRep_Defense(const FGameplayAttributeData& OldDefense);

	// ========================================
	// 移动属性
	// ========================================

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Movement", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, MoveSpeed)

	UFUNCTION()
	virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);

	// ========================================
	// 能量属性
	// ========================================

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Energy", ReplicatedUsing = OnRep_MaxEnergy)
	FGameplayAttributeData MaxEnergy;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, MaxEnergy)

	UFUNCTION()
	virtual void OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy);

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Energy", ReplicatedUsing = OnRep_CurrentEnergy)
	FGameplayAttributeData CurrentEnergy;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, CurrentEnergy)

	UFUNCTION()
	virtual void OnRep_CurrentEnergy(const FGameplayAttributeData& OldCurrentEnergy);

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Energy", ReplicatedUsing = OnRep_EnergyRegen)
	FGameplayAttributeData EnergyRegen;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, EnergyRegen)

	UFUNCTION()
	virtual void OnRep_EnergyRegen(const FGameplayAttributeData& OldEnergyRegen);

	// ========================================
	// 暴击属性
	// ========================================

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Critical", ReplicatedUsing = OnRep_CriticalRate)
	FGameplayAttributeData CriticalRate;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, CriticalRate)

	UFUNCTION()
	virtual void OnRep_CriticalRate(const FGameplayAttributeData& OldCriticalRate);

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Critical", ReplicatedUsing = OnRep_CriticalMultiplier)
	FGameplayAttributeData CriticalMultiplier;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, CriticalMultiplier)

	UFUNCTION()
	virtual void OnRep_CriticalMultiplier(const FGameplayAttributeData& OldCriticalMultiplier);

	// ========================================
	// 护盾属性
	// ========================================

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Shield", ReplicatedUsing = OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, MaxShield)

	UFUNCTION()
	virtual void OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield);

	UPROPERTY(BlueprintReadOnly, Category = "Battle|Shield", ReplicatedUsing = OnRep_CurrentShield)
	FGameplayAttributeData CurrentShield;
	BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, CurrentShield)

	UFUNCTION()
	virtual void OnRep_CurrentShield(const FGameplayAttributeData& OldCurrentShield);
};
