// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：GameCore 层编译期常量，集中管理跨模块共享的资产路径与键名。
- 阅读重点：本文件仅承载编译期安全边界常量，不承载业务可变数值；可变数值应通过 DataAsset/DataTable/DeveloperSettings 驱动。
- 修改提示：新增常量前请确认其属于"无法数据化的低层技术常量"或"协议键名"；业务数值请走数据资产。
*/


#pragma once

#include "CoreMinimal.h"

/**
 * DBA GameCore 层常量命名空间
 *
 * 仅承载编译期安全边界常量：
 * - 资产根路径（用于资产系统初始化、AssetManager 配置）
 * - 地图路径（用于 GameCore 层会话/登录流程的默认地图）
 * - 协议键名（用于配置文件解析、网络协议字段）
 *
 * 业务数值（伤害系数、冷却时间、UI 文案等）不应放在此处，
 * 应通过 DataAsset / DataTable / DeveloperSettings 驱动。
 */
namespace DBAConstantsCore
{
	/**
	 * 默认大厅地图路径
	 *
	 * GameCore 层会话/登录流程在没有配置覆盖时使用此默认值。
	 * DivineBeastsArena 层的 DBAConstants::Map_MainLobby 应引用此常量，
	 * 避免路径分歧。
	 *
	 * 已废弃：前台地图由 UDBAFrontendSettings 的软引用配置；该常量仅供尚未迁移的大厅代码兼容使用。
	 */
	constexpr const TCHAR* Map_MainLobby = TEXT("/Game/Lobby/Maps/MainLobby");

	/**
	 * 默认新手村地图路径
	 *
	 * 用于首次进入游戏的新手引导流程。
	 */
	constexpr const TCHAR* Map_NewbieVillage = TEXT("/Game/Lobby/Maps/NewbieVillage");

	/**
	 * 默认练习场地图路径
	 *
	 * 用于单人练习与英雄试玩。
	 */
	constexpr const TCHAR* Map_Practice = TEXT("/Game/Practice/Maps/Practice_Training");
};
