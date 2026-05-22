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

/**
 * SaveGame 版本管理
 *
 * 版本策略：
 * - SchemaVersion：存档结构版本，结构变化时递增
 * - DataVersion：数据版本，数据格式变化时递增
 *
 * 版本兼容性：
 * - 向后兼容：新版本可以读取旧版本存档
 * - 向前兼容：旧版本无法读取新版本存档
 *
 * 迁移策略：
 * - 检测版本不匹配时触发迁移
 * - 迁移失败时使用默认数据
 * - 保留原始存档作为备份
 *
 * 损坏处理：
 * - 检测存档损坏时尝试修复
 * - 修复失败时使用默认数据
 * - 记录损坏日志用于分析
 */
namespace DBASaveGameVersions
{
	/**
	 * 存档结构版本
	 * 当存档结构发生变化时递增
	 */
	constexpr int32 SCHEMA_VERSION = 1;

	/**
	 * 数据版本
	 * 当数据格式发生变化时递增
	 */
	constexpr int32 DATA_VERSION = 1;

	/**
	 * 最小支持的结构版本
	 * 低于此版本的存档无法迁移
	 */
	constexpr int32 MIN_SUPPORTED_SCHEMA_VERSION = 1;

	/**
	 * 最小支持的数据版本
	 * 低于此版本的存档无法迁移
	 */
	constexpr int32 MIN_SUPPORTED_DATA_VERSION = 1;

	/**
	 * 存档槽名称
	 */
	namespace SlotNames
	{
		/** 账户存档槽 */
		static const FString ACCOUNT_SLOT = TEXT("DBAAccount");

		/** Profile 存档槽 */
		static const FString PROFILE_SLOT = TEXT("DBAProfile");

		/** 备份存档槽后缀 */
		static const FString BACKUP_SUFFIX = TEXT("_Backup");
	}

	/**
	 * 检查版本是否兼容
	 *
	 * @param SchemaVersion 存档结构版本
	 * @param DataVersion 存档数据版本
	 * @return 是否兼容
	 */
	inline bool IsVersionCompatible(int32 SchemaVersion, int32 DataVersion)
	{
		return SchemaVersion >= MIN_SUPPORTED_SCHEMA_VERSION
			&& SchemaVersion <= SCHEMA_VERSION
			&& DataVersion >= MIN_SUPPORTED_DATA_VERSION
			&& DataVersion <= DATA_VERSION;
	}

	/**
	 * 检查是否需要迁移
	 *
	 * @param SchemaVersion 存档结构版本
	 * @param DataVersion 存档数据版本
	 * @return 是否需要迁移
	 */
	inline bool NeedsMigration(int32 SchemaVersion, int32 DataVersion)
	{
		return SchemaVersion < SCHEMA_VERSION || DataVersion < DATA_VERSION;
	}
}
