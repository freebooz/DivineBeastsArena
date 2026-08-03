// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// GameplayEffect - 火元素共鸣

#include "GameDBA/Gameplay/GAS/Effects/DBAGE_Fire_Resonance.h"
UDBAGE_Fire_Resonance::UDBAGE_Fire_Resonance()
{
	// 共鸣数值必须在运行时通过数据资产解析，不能在 CDO 构造阶段同步加载数据表。
	// 原实现会将控制时长和护盾加成错误写入 CurrentHealth，因此不在此静态 GE 上配置 Modifier。
}
