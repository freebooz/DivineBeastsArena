// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 战斗系统类型定义

#pragma once

#include "CoreMinimal.h"
#include "DBACombatTypes.generated.h"

/**
 * EDBATargetLockStrategy
 * 目标锁定策略枚举
 */
UENUM(BlueprintType)
enum class EDBATargetLockStrategy : uint8
{
	Smart      UMETA(DisplayName = "Smart", Tooltip = "智能锁定（威胁最高）"),
	LowestHP   UMETA(DisplayName = "LowestHP", Tooltip = "血量锁定（最低血量）"),
	Nearest    UMETA(DisplayName = "Nearest", Tooltip = "距离锁定（最近目标）")
};

/**
 * EDADeathState
 * 角色死亡状态枚举
 */
UENUM(BlueprintType)
enum class EDADeathState : uint8
{
	Alive       UMETA(DisplayName = "Alive", Tooltip = "存活状态"),
	Dying       UMETA(DisplayName = "Dying", Tooltip = "死亡中（播放死亡动画）"),
	Dead        UMETA(DisplayName = "Dead", Tooltip = "死亡状态"),
	Respawning  UMETA(DisplayName = "Respawning", Tooltip = "复活中")
};