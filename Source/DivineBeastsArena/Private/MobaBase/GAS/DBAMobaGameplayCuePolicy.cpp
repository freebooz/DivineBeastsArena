// Copyright FreeboozStudio. All Rights Reserved.

#include "MobaBase/GAS/DBAMobaGameplayCuePolicy.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

bool UDBAMobaGameplayCuePolicy::ShouldExecuteGameplayCue_Implementation(FGameplayTag CueTag) const
{
	// 默认实现：检查是否为 Dedicated Server
	// Dedicated Server 不执行表现相关的 Cue
	if (IsDedicatedServer())
	{
		return false;
	}

	// 默认允许执行 Cue
	return true;
}

bool UDBAMobaGameplayCuePolicy::IsDedicatedServer() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return World->GetNetMode() == NM_DedicatedServer;
}

bool UDBAMobaGameplayCuePolicy::IsLocalPlayer_Implementation() const
{
	// 默认返回 false
	// 派生类应根据具体上下文重写此方法
	return false;
}