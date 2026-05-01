// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - 羊W技能

#include "DBA/GAS/Effects/DBAGE_Goat_W.h"
#include "DBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "Engine/DataTable.h"
#include "Data/DBADataAsset.h"

UDBAGE_Goat_W::UDBAGE_Goat_W()
{
	// 从技能数据表加载配置
	UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
	if (SkillTable)
	{
		static const FString ContextString = TEXT("DBAGE_Goat_W");
		FDBASkillDataRow* SkillRow = SkillTable->FindRow<FDBASkillDataRow>(FName(TEXT("Goat_W")), ContextString, false);
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
