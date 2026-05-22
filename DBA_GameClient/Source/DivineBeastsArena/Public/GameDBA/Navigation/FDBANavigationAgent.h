// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 导航代理配置结构

#pragma once

#include "CoreMinimal.h"
#include "FDBANavigationAgent.generated.h"

/**
 * FDBA NavigationAgent
 * 导航代理配置结构
 * 用于配置NavMesh代理的尺寸和行为
 */
USTRUCT(BlueprintType)
struct FDBA_NavigationAgent
{
	GENERATED_BODY()

public:
	/** 代理半径 (NavMesh Agent Radius) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Agent", meta = (UIMin = 10.0, UIMax = 100.0))
	float AgentRadius = 34.0f;

	/** 代理高度 (NavMesh Agent Height) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Agent", meta = (UIMin = 50.0, UIMax = 250.0))
	float AgentHeight = 144.0f;

	/** 可行走地面Z值阈值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation|Agent", meta = (UIMin = 0.0, UIMax = 1.0))
	float WalkableFloorZ = 0.7f;
};