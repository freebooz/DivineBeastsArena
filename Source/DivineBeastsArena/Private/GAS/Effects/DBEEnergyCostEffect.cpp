// Copyright FreeboozStudio. All Rights Reserved.

#include "GAS/Effects/DBEEnergyCostEffect.h"
#include "GAS/Attributes/DBABattleAttributeSet.h"

UDBEEnergyCostEffect::UDBEEnergyCostEffect()
{
	// 即时效果：立即应用并结束
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// 能量消耗修饰符
	// 配置为减少 CurrentEnergy
	FGameplayModifierInfo EnergyModifier;

	// 使用 UClass::FindPropertyByName 获取属性
	if (UClass* AttributeClass = UDBABattleAttributeSet::StaticClass())
	{
		if (FProperty* EnergyProperty = AttributeClass->FindPropertyByName(FName(TEXT("CurrentEnergy"))))
		{
			EnergyModifier.Attribute.SetUProperty(EnergyProperty);
		}
	}

	EnergyModifier.ModifierOp = EGameplayModOp::Additive;
	// 注意：此处的 Magnitude 会被 Ability 的 CostGameplayEffect 配置覆盖
	// 正确做法：在 Ability 的 CostGameplayEffect 属性中设置 CostMagnitude
	// 此 GE 作为模板，Magnitude 值仅为默认值
	EnergyModifier.ModifierMagnitude = FScalableFloat(-1.0f);

	Modifiers.Add(EnergyModifier);
}