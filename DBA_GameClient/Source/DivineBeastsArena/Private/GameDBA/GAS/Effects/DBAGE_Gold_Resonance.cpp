// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// GameplayEffect - 閲戝叡楦ｅ厓绱犲叡楦?
#include "GameDBA/GAS/Effects/DBAGE_Gold_Resonance.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "Engine/DataTable.h"

UDBAGE_Gold_Resonance::UDBAGE_Gold_Resonance()
{
	// 浠庡厓绱犲叡楦ｆ暟鎹〃鍔犺浇閰嶇疆
	UDataTable* ResonanceTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Elements/DBAElementResonanceRowe.DBAElementResonanceRowe'"));
	if (ResonanceTable)
	{
		static const FString ContextString = TEXT("DBAGE_Gold_Resonance");
		FDBAElementResonanceRow* ResonanceRow = ResonanceTable->FindRow<FDBAElementResonanceRow>(FName(TEXT("Gold")), ContextString, false);
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

