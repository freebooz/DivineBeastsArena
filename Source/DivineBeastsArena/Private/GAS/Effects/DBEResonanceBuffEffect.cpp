// Copyright FreeboozStudio. All Rights Reserved.

#include "GAS/Effects/DBEResonanceBuffEffect.h"
#include "GAS/Attributes/DBABattleAttributeSet.h"

UDBEResonanceBuffEffect::UDBEResonanceBuffEffect()
{
	// 持续效果：持续到手动移除
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(0.0f);  // 0 表示永久，需要手动移除

	// 无冷却
	PeriodicInterval = 0.0f;
	bExecutePeriodicEffectOnApplication = false;

	// 复制策略：复制到所有相关客户端
	ReplicationPolicy = EGameplayEffectReplicationPolicy::ReplicateInstanced;

	// 堆叠策略：替换同类型效果
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
	StackLimitCount = 1;
	StackPeriodResetOnApplication = false;

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

	// 伤害加成修饰符
	FGameplayModifierInfo DamageInfo;
	DamageInfo.Attribute.SetUProperty(FindPropertyByName(FName(TEXT("AttackPower"))));
	DamageInfo.ModifierOp = EGameplayModOp::Additive;
	DamageInfo.ModifierMagnitude = FScalableFloat(DamageBonus);
	Modifiers.Add(DamageInfo);

	// 防御加成修饰符
	FGameplayModifierInfo DefenseInfo;
	DefenseInfo.Attribute.SetUProperty(FindPropertyByName(FName(TEXT("Defense"))));
	DefenseInfo.ModifierOp = EGameplayModOp::Additive;
	DefenseInfo.ModifierMagnitude = FScalableFloat(DefenseBonus);
	Modifiers.Add(DefenseInfo);

	// 生命值加成修饰符
	FGameplayModifierInfo HealthInfo;
	HealthInfo.Attribute.SetUProperty(FindPropertyByName(FName(TEXT("MaxHealth"))));
	HealthInfo.ModifierOp = EGameplayModOp::Additive;
	HealthInfo.ModifierMagnitude = FScalableFloat(HealthBonus);
	Modifiers.Add(HealthInfo);

	// 护盾值加成修饰符（通过 MaxShield 属性实现共鸣护盾加成）
	FGameplayModifierInfo ShieldInfo;
	ShieldInfo.Attribute.SetUProperty(FindPropertyByName(FName(TEXT("MaxShield"))));
	ShieldInfo.ModifierOp = EGameplayModOp::Additive;
	ShieldInfo.ModifierMagnitude = FScalableFloat(ShieldBonus);
	Modifiers.Add(ShieldInfo);
}
