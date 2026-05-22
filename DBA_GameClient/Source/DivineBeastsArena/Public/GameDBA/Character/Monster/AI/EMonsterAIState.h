// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 怪物AI状态枚举

#pragma once

#include "CoreMinimal.h"
#include "EMonsterAIState.generated.h"

/**
 * EMonsterAIState
 * 怪物AI状态枚举
 */
UENUM(BlueprintType)
enum class EMonsterAIState : uint8
{
	Idle    UMETA(DisplayName = "Idle", Tooltip = "原地待机状态"),
	Patrol  UMETA(DisplayName = "Patrol", Tooltip = "巡逻状态"),
	Chase   UMETA(DisplayName = "Chase", Tooltip = "追击状态"),
	Attack  UMETA(DisplayName = "Attack", Tooltip = "攻击状态"),
	Return  UMETA(DisplayName = "Return", Tooltip = "返回状态")
};