// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Frontend/Core/DBAFrontendContracts.h"

/** 前台状态转换规则。FlowSubsystem 是唯一可提交状态变化的对象。 */
namespace DBAFrontendStateMachine
{
	DIVINEBEASTSARENA_API bool CanTransition(EDBAFrontendState From, EDBAFrontendState To);
	DIVINEBEASTSARENA_API bool IsCharacterCreationState(EDBAFrontendState State);
}
