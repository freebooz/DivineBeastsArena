// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - 狗被动技能

#include "DBA/GAS/Effects/DBAGE_Dog_Passive.h"
#include "DBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "Engine/DataTable.h"

UDBAGE_Dog_Passive::UDBAGE_Dog_Passive()
{
	// 从技能数据表加载配置
	UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
	if (SkillTable)
	{
		static const FString ContextString = TEXT("DBAGE_Dog_Passive");
		FDBASkillDataRow* SkillRow = SkillTable->FindRow<FDBASkillDataRow>(FName(TEXT("Dog_Passive")), ContextString, false);
		if (SkillRow)
		{
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
		}
	}
}
