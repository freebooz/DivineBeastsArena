// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 蛇W技能

#include "DBA/GAS/Cues/DBACue_Snake_W.h"

UDBACue_Snake_W::UDBACue_Snake_W()
{
	// 默认配置
	bAutoDestroyOnOwnerRemoved = true;
	bOnlyRelevantToOwner = false;
	RollbackPolicy = ECueRollback::CanRollback;
}

bool UDBACue_Snake_W::OnExecuteGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 播放施法特效
	// 示例: PlayEffects(Target, Parameters);

	return true;
}

void UDBACue_Snake_W::OnActiveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 激活状态特效
}

void UDBACue_Snake_W::OnRemoveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{
	// TODO: 移除特效
}
