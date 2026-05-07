// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - 鍦熷叡楦ｅ厓绱犲叡楦?
#include "GameDBA/GAS/Effects/DBAGE_Earth_Resonance.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "Engine/DataTable.h"

UDBAGE_Earth_Resonance::UDBAGE_Earth_Resonance()
{
	// 浠庡厓绱犲叡楦ｆ暟鎹〃鍔犺浇閰嶇疆
	UDataTable* ResonanceTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Elements/ElementResonanceTable.ElementResonanceTable'"));
	if (ResonanceTable)
	{
		static const FString ContextString = TEXT("DBAGE_Earth_Resonance");
		FDBAElementResonanceRow* ResonanceRow = ResonanceTable->FindRow<FDBAElementResonanceRow>(FName(TEXT("Earth")), ContextString, false);
		if (ResonanceRow)
		{
			// 鎺у埗鏃堕棿鍔犳垚
			if (ResonanceRow->ControlTimeBonus > 0)
			{
				FGameplayModifierInfo ControlMod;
				ControlMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
				ControlMod.ModifierOp = EGameplayModOp::Additive;
				ControlMod.ModifierMagnitude = FScalableFloat(ResonanceRow->ControlTimeBonus);
				Modifiers.Add(ControlMod);
			}

			// 鎶ょ浘鍊煎姞鎴?			if (ResonanceRow->ShieldBonus > 0)
			{
				FGameplayModifierInfo ShieldMod;
				ShieldMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
				ShieldMod.ModifierOp = EGameplayModOp::Additive;
				ShieldMod.ModifierMagnitude = FScalableFloat(ResonanceRow->ShieldBonus);
				Modifiers.Add(ShieldMod);
			}
		}
	}
}

