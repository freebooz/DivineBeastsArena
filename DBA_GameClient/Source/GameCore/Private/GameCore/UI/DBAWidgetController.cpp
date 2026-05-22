// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameCore/UI/DBAWidgetController.h"

/**
 * 构造函数
 * 初始化 Widget 控制器基类
 */
UDBAWidgetController::UDBAWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 初始化控制器
 * 设置 bIsInitialized 为 true，表示控制器已准备好
 * 由持有控制器的 Widget 或Subsystem 调用
 */
void UDBAWidgetController::InitializeController()
{
	bIsInitialized = true;
}

/**
 * 重置控制器状态
 * 设置 bIsInitialized 为 false，表示控制器已失效
 * 用于 Widget 销毁或控制器切换时清理状态
 */
void UDBAWidgetController::ResetController()
{
	bIsInitialized = false;
}
