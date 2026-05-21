// Copyright Freebooz Games, Inc. All Rights Reserved.
// 泛化GameplayEffect类实现

#include "GameDBA/GAS/Effects/DBAGE_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"

UDBAGE_Generic::UDBAGE_Generic()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		LoadAndApplyModifiers();
	}
}

void UDBAGE_Generic::LoadAndApplyModifiers()
{
	if (!SkillTable || SkillID.IsNone())
	{
		return;
	}

	static const FString ContextString = TEXT("DBAGE_Generic");
	FDBASkillDataRow* SkillRow = SkillTable->FindRow<FDBASkillDataRow>(SkillID, ContextString, false);

	if (!SkillRow)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAGE_Generic] 未找到技能配置: %s"), *SkillID.ToString());
		return;
	}

	// 伤害修饰符
	if (SkillRow->BaseDamage > 0)
	{
		FGameplayModifierInfo DamageMod;
		DamageMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
		DamageMod.ModifierOp = EGameplayModOp::Additive;
		DamageMod.ModifierMagnitude = FScalableFloat(SkillRow->BaseDamage);
		Modifiers.Add(DamageMod);
	}

	// 治疗修饰符
	if (SkillRow->HealAmount > 0)
	{
		FGameplayModifierInfo HealMod;
		HealMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
		HealMod.ModifierOp = EGameplayModOp::Additive;
		HealMod.ModifierMagnitude = FScalableFloat(SkillRow->HealAmount);
		Modifiers.Add(HealMod);
	}

	// 护盾修饰符
	if (SkillRow->ShieldValue > 0)
	{
		FGameplayModifierInfo ShieldMod;
		ShieldMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
		ShieldMod.ModifierOp = EGameplayModOp::Additive;
		ShieldMod.ModifierMagnitude = FScalableFloat(SkillRow->ShieldValue);
		Modifiers.Add(ShieldMod);
	}

	// 设置持续时间
	if (SkillRow->ControlTime > 0)
	{
		DurationPolicy = EGameplayEffectDurationType::HasDuration;
		Period = 0.0f;
		DurationMagnitude = FScalableFloat(SkillRow->ControlTime);
	}

	UE_LOG(LogDBACombat, Log, TEXT("[DBAGE_Generic] 已加载技能配置：技能=%s 基础伤害=%.1f 治疗量=%.1f 护盾值=%.1f"),
		*SkillID.ToString(), SkillRow->BaseDamage, SkillRow->HealAmount, SkillRow->ShieldValue);
}
