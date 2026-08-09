// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

/**
 * 核心日志域
 * 用于记录引擎初始化、模块加载、全局状态变更
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBACore, Log, All);

/**
 * 前台流程日志域
 * 用于记录登录、大厅、队伍、匹配、英雄选择等前台业务流程
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAFrontend, Log, All);

/** 在线服务日志域：认证、HTTP、会话与服务目录。严禁输出 Token、Password、RefreshToken 或 GameTicket。 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAOnline, Log, All);

/** 角色日志域：角色列表、选择、创建草稿及服务端校验结果。 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBACharacter, Log, All);

/** 预览日志域：角色预览场景、SceneCapture 与软资源加载。 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAPreview, Log, All);

/**
 * 匹配系统日志域
 * 用于记录匹配队列、准备检查、房间创建、Travel 流程
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAMatch, Log, All);

/**
 * 战斗系统日志域
 * 用于记录 GAS、技能释放、伤害计算、连锁、共鸣、生肖大招
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBACombat, Log, All);

/**
 * UI 系统日志域
 * 用于记录 Widget 创建、HUD 更新、输入绑定、布局切换
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAUI, Log, All);

/**
 * 数据系统日志域
 * 用于记录 DataTable、DataAsset、配置加载、数据校验
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAData, Log, All);

/**
 * 网络系统日志域
 * 用于记录复制、RPC、连接、断线、延迟、带宽
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBANetwork, Log, All);

/**
 * 数据校验日志域
 * 用于记录 DataValidation、资源完整性检查、配置冲突
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAValidation, Log, All);

/**
 * AI 系统日志域
 * 用于记录 Minion、Jungle、Monster、Turret、AI 行为树、导航
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAAI, Log, All);

/**
 * 遥测日志域
 * 用于记录可选的外部 Monitoring 上报、事件采集、性能指标
 * 此日志域不影响游戏核心逻辑
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBATelemetry, Log, All);

/**
 * GameOps 日志域
 * 用于记录可选的外部 GameOps 可见性数据读取、配置下发
 * 此日志域不影响游戏核心逻辑
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAGameOps, Log, All);

/**
 * VFX 日志域
 * 用于记录视觉特效播放、加载、回退、异步预加载状态
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAVFX, Log, All);

/**
 * SFX 日志域
 * 用于记录音效播放、加载、回退、异步预加载状态
 */
GAMECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDBASFX, Log, All);
