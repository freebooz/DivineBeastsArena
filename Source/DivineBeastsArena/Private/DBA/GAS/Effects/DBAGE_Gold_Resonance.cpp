// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - 金共鸣元素共鸣

#include "DBA/GAS/Effects/DBAGE_Gold_Resonance.h"
#include "DBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "Engine/DataTable.h"

UDBAGE_Gold_Resonance::UDBAGE_Gold_Resonance()
{
	// 从元素共鸣数据表加载配置
	UDataTable* ResonanceTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Elements/ElementResonanceTable.ElementResonanceTable'"));
	if (ResonanceTable)
	{
		static const FString ContextString = TEXT("DBAGE_Gold_Resonance");
		FDBADBElemenetResonanceRow* ResonanceRow = ResonanceTable->FindRow<FDBADBElemenetResonanceRow>(FName(TEXT("Gold")), ContextString, false);
		if (ResonanceRow)
		{
			// 控制时间加成
			if (ResonanceRow->ControlTimeBonus > 0)
			{
				FGameplayModifierInfo ControlMod;
				ControlMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
				ControlMod.ModifierOp = EGameplayModOp::Additive;
				ControlMod.ModifierMagnitude = FScalableFloat(ResonanceRow->ControlTimeBonus);
				Modifiers.Add(ControlMod);
			}

			// 护盾值加成
			if (ResonanceRow->ShieldBonus > 0)
			{
				FGameplayModifierInfo ShieldMod;
				ShieldMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
				ShieldMod.ModifierOp = EGameplayModOp::Additive;
				ShieldMod.ModifierMagnitude = FScalableFloat(ResonanceRow->ShieldBonus);
				Modifiers.Add(ShieldMod);
			}

			// 设置持续时间
			if (ResonanceRow->Duration > 0)
			{
				DurationPolicy = EGameplayEffectDurationType::Infinite;
			}
		}
	}
}
