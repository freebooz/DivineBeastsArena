// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 鸡E技能

#include "DBA/GAS/Cues/DBACue_Rooster_E.h"

UDBACue_Rooster_E::UDBACue_Rooster_E()
{
	// 默认配置
	bAutoDestroyOnOwnerRemoved = true;
	bOnlyRelevantToOwner = false;
	RollbackPolicy = ECueRollback::CanRollback;
}

bool UDBACue_Rooster_E::OnExecuteGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 播放施法特效
	// 示例: PlayEffects(Target, Parameters);

	return true;
}

void UDBACue_Rooster_E::OnActiveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 激活状态特效
}

void UDBACue_Rooster_E::OnRemoveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 移除特效
}
