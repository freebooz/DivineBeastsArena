// Copyright FreeboozStudio. All Rights Reserved.
// 接口聚合头文件 - 为保持向后兼容而保留
// 新代码应直接包含具体接口头文件

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameDBA/Core/DBAResultTypes.h"
#include "GameDBA/Core/Interfaces/DBAValidatableInterface.h"
#include "GameDBA/Core/Interfaces/DBAVersionableInterface.h"
#include "GameDBA/Core/Interfaces/DBADebugDescribableInterface.h"
#include "GameDBA/Core/Interfaces/DBAAvailabilityInterface.h"
#include "GameDBA/Core/Interfaces/DBAInteractableInterface.h"
#include "GameDBA/Core/Interfaces/DBATeamAgentInterface.h"
#include "GameDBA/Core/Interfaces/DBATargetableInterface.h"
#include "GameDBA/Core/Interfaces/DBATelemetryInterface.h"
#include "GameDBA/Core/Interfaces/DBAExternalServiceDegradableInterface.h"

/**
 * DBAInterfacesCore.h - 接口聚合头文件
 *
 * 所有接口已拆分到 GameDBA/Core/Interfaces/ 目录:
 * - DBAValidatableInterface.h
 * - DBAVersionableInterface.h
 * - DBADebugDescribableInterface.h
 * - DBAAvailabilityInterface.h
 * - DBAInteractableInterface.h
 * - DBATeamAgentInterface.h
 * - DBATargetableInterface.h
 * - DBATelemetryInterface.h
 * - DBAExternalServiceDegradableInterface.h
 *
 * 此文件仅为向后兼容保留，新代码应直接包含具体接口。
 */
