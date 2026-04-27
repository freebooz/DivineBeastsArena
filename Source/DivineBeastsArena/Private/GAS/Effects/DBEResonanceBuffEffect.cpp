// Copyright FreeboozStudio. All Rights Reserved.
// 共鸣护盾增益效果实现 - 根据共鸣等级提供属性加成

#include "GAS/Effects/DBEResonanceBuffEffect.h"
#include "GAS/Attributes/DBABattleAttributeSet.h"

// 构造函数 - 初始化共鸣增益效果
UDBEResonanceBuffEffect::UDBEResonanceBuffEffect()
{
	// 持续效果：持续到手动移除
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(0.0f);  // 0 表示永久，需要手动移除

	// 默认 Lv.1 配置
	ConfigureForResonanceLevel(1);
}

// ConfigureForResonanceLevel - 根据共鸣等级配置效果参数
void UDBEResonanceBuffEffect::ConfigureForResonanceLevel(int32 Level)
{
	// 限制共鸣等级在有效范围内（1-4级）
	ResonanceLevel = FMath::Clamp(Level, 1, 4);

	// 清空现有修饰符
	Modifiers.Empty();

	// 根据共鸣等级计算加成值
	float DamageBonus = 0.0f;
	float DefenseBonus = 0.0f;
	float HealthBonus = 0.0f;
	float ShieldBonus = 0.0f;

	// 根据共鸣等级设置对应的属性加成数值
	switch (ResonanceLevel)
	{
	case 1:
		DamageBonus = 5.0f;
		DefenseBonus = 2.0f;
		HealthBonus = 3.0f;
		ShieldBonus = 5.0f;
		break;
	case 2:
		DamageBonus = 10.0f;
		DefenseBonus = 4.0f;
		HealthBonus = 6.0f;
		ShieldBonus = 10.0f;
		break;
	case 3:
		DamageBonus = 15.0f;
		DefenseBonus = 6.0f;
		HealthBonus = 9.0f;
		ShieldBonus = 15.0f;
		break;
	case 4:
		DamageBonus = 20.0f;
		DefenseBonus = 8.0f;
		HealthBonus = 12.0f;
		ShieldBonus = 20.0f;
		break;
	default:
		break;
	}

	// 获取属性类用于查找属性
	UClass* AttributeClass = UDBABattleAttributeSet::StaticClass();

	// 创建伤害加成修饰符（攻击力）
	FGameplayModifierInfo DamageInfo;
	if (FProperty* AttackProperty = AttributeClass->FindPropertyByName(FName(TEXT("AttackPower"))))
	{
		DamageInfo.Attribute = AttackProperty;
	}
	DamageInfo.ModifierOp = EGameplayModOp::Additive;
	DamageInfo.ModifierMagnitude = FScalableFloat(DamageBonus);
	Modifiers.Add(DamageInfo);

	// 创建防御加成修饰符（防御力）
	FGameplayModifierInfo DefenseInfo;
	if (FProperty* DefenseProperty = AttributeClass->FindPropertyByName(FName(TEXT("Defense"))))
	{
		DefenseInfo.Attribute = DefenseProperty;
	}
	DefenseInfo.ModifierOp = EGameplayModOp::Additive;
	DefenseInfo.ModifierMagnitude = FScalableFloat(DefenseBonus);
	Modifiers.Add(DefenseInfo);

	// 创建生命值加成修饰符（最大生命）
	FGameplayModifierInfo HealthInfo;
	if (FProperty* MaxHealthProperty = AttributeClass->FindPropertyByName(FName(TEXT("MaxHealth"))))
	{
		HealthInfo.Attribute = MaxHealthProperty;
	}
	HealthInfo.ModifierOp = EGameplayModOp::Additive;
	HealthInfo.ModifierMagnitude = FScalableFloat(HealthBonus);
	Modifiers.Add(HealthInfo);

	// 创建护盾值加成修饰符（通过 MaxShield 属性实现共鸣护盾加成）
	FGameplayModifierInfo ShieldInfo;
	if (FProperty* MaxShieldProperty = AttributeClass->FindPropertyByName(FName(TEXT("MaxShield"))))
	{
		ShieldInfo.Attribute = MaxShieldProperty;
	}
	ShieldInfo.ModifierOp = EGameplayModOp::Additive;
	ShieldInfo.ModifierMagnitude = FScalableFloat(ShieldBonus);
	Modifiers.Add(ShieldInfo);
}