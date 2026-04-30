// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 猴终极技能

#include "DBA/GAS/Cues/DBACue_Monkey_R.h"

UDBACue_Monkey_R::UDBACue_Monkey_R()
{
	// 默认配置
	bAutoDestroyOnOwnerRemoved = true;
	bOnlyRelevantToOwner = false;
	RollbackPolicy = ECueRollback::CanRollback;
}

bool UDBACue_Monkey_R::OnExecuteGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 播放施法特效
	// 示例: PlayEffects(Target, Parameters);

	return true;
}

void UDBACue_Monkey_R::OnActiveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 激活状态特效
}

void UDBACue_Monkey_R::OnRemoveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 移除特效
}
