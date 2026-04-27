// Copyright FreeboozStudio. All Rights Reserved.

#include "GAS/Effects/DBEResonanceBuffEffect.h"
#include "GAS/Attributes/DBABattleAttributeSet.h"

UDBEResonanceBuffEffect::UDBEResonanceBuffEffect()
{
	// 持续效果：持续到手动移除
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(0.0f);  // 0 表示永久，需要手动移除

	// 默认 Lv.1 配置
	ConfigureForResonanceLevel(1);
}

void UDBEResonanceBuffEffect::ConfigureForResonanceLevel(int32 Level)
{
	ResonanceLevel = FMath::Clamp(Level, 1, 4);

	// 清空现有修饰符
	Modifiers.Empty();

	// 根据共鸣等级计算加成值
	float DamageBonus = 0.0f;
	float DefenseBonus = 0.0f;
	float HealthBonus = 0.0f;
	float ShieldBonus = 0.0f;

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

	// 获取属性类
	UClass* AttributeClass = UDBABattleAttributeSet::StaticClass();

	// 伤害加成修饰符
	FGameplayModifierInfo DamageInfo;
	if (FProperty* AttackProperty = AttributeClass->FindPropertyByName(FName(TEXT("AttackPower"))))
	{
		DamageInfo.Attribute.SetUProperty(AttackProperty);
	}
	DamageInfo.ModifierOp = EGameplayModOp::Additive;
	DamageInfo.ModifierMagnitude = FScalableFloat(DamageBonus);
	Modifiers.Add(DamageInfo);

	// 防御加成修饰符
	FGameplayModifierInfo DefenseInfo;
	if (FProperty* DefenseProperty = AttributeClass->FindPropertyByName(FName(TEXT("Defense"))))
	{
		DefenseInfo.Attribute.SetUProperty(DefenseProperty);
	}
	DefenseInfo.ModifierOp = EGameplayModOp::Additive;
	DefenseInfo.ModifierMagnitude = FScalableFloat(DefenseBonus);
	Modifiers.Add(DefenseInfo);

	// 生命值加成修饰符
	FGameplayModifierInfo HealthInfo;
	if (FProperty* MaxHealthProperty = AttributeClass->FindPropertyByName(FName(TEXT("MaxHealth"))))
	{
		HealthInfo.Attribute.SetUProperty(MaxHealthProperty);
	}
	HealthInfo.ModifierOp = EGameplayModOp::Additive;
	HealthInfo.ModifierMagnitude = FScalableFloat(HealthBonus);
	Modifiers.Add(HealthInfo);

	// 护盾值加成修饰符（通过 MaxShield 属性实现共鸣护盾加成）
	FGameplayModifierInfo ShieldInfo;
	if (FProperty* MaxShieldProperty = AttributeClass->FindPropertyByName(FName(TEXT("MaxShield"))))
	{
		ShieldInfo.Attribute.SetUProperty(MaxShieldProperty);
	}
	ShieldInfo.ModifierOp = EGameplayModOp::Additive;
	ShieldInfo.ModifierMagnitude = FScalableFloat(ShieldBonus);
	Modifiers.Add(ShieldInfo);
}