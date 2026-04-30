// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 马被动技能

#include "DBA/GAS/Cues/DBACue_Horse_Passive.h"

UDBACue_Horse_Passive::UDBACue_Horse_Passive()
{
	// 默认配置
	bAutoDestroyOnOwnerRemoved = true;
	bOnlyRelevantToOwner = false;
	RollbackPolicy = ECueRollback::CanRollback;
}

bool UDBACue_Horse_Passive::OnExecuteGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 播放施法特效
	// 示例: PlayEffects(Target, Parameters);

	return true;
}

void UDBACue_Horse_Passive::OnActiveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 激活状态特效
}

void UDBACue_Horse_Passive::OnRemoveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 移除特效
}
