// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

/**
 * 核心日志域
 * 用于记录引擎初始化、模块加载、全局状态变更
 */
DIVINEBEASTSARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogDBACore, Log, All);

/**
 * 前台流程日志域
 * 用于记录登录、大厅、队伍、匹配、英雄选择等前台业务流程
 */
DIVINEBEASTSARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAFrontend, Log, All);

/**
 * 匹配系统日志域
 * 用于记录匹配队列、准备检查、房间创建、Travel 流程
 */
DIVINEBEASTSARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAMatch, Log, All);

/**
 * 战斗系统日志域
 * 用于记录 GAS、技能释放、伤害计算、连锁、共鸣、生肖大招
 */
DIVINEBEASTSARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogDBACombat, Log, All);

/**
 * UI 系统日志域
 * 用于记录 Widget 创建、HUD 更新、输入绑定、布局切换
 */
DIVINEBEASTSARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAUI, Log, All);

/**
 * 数据系统日志域
 * 用于记录 DataTable、DataAsset、配置加载、数据校验
 */
DIVINEBEASTSARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAData, Log, All);

/**
 * 网络系统日志域
 * 用于记录复制、RPC、连接、断线、延迟、带宽
 */
DIVINEBEASTSARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogDBANetwork, Log, All);

/**
 * 数据校验日志域
 * 用于记录 DataValidation、资源完整性检查、配置冲突
 */
DIVINEBEASTSARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAValidation, Log, All);

/**
 * AI 系统日志域
 * 用于记录 Minion、Jungle、Monster、Turret、AI 行为树、导航
 */
DIVINEBEASTSARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAAI, Log, All);

/**
 * 遥测日志域
 * 用于记录可选的外部 Monitoring 上报、事件采集、性能指标
 * 此日志域不影响游戏核心逻辑
 */
DIVINEBEASTSARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogDBATelemetry, Log, All);

/**
 * GameOps 日志域
 * 用于记录可选的外部 GameOps 可见性数据读取、配置下发
 * 此日志域不影响游戏核心逻辑
 */
DIVINEBEASTSARENA_API DECLARE_LOG_CATEGORY_EXTERN(LogDBAGameOps, Log, All);