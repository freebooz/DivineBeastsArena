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
#include "UObject/Interface.h"
#include "DBAVersionableInterface.generated.h"

/**
 * 可版本化对象接口
 * 用于 SaveGame、Profile、配置文件的版本管理和迁移
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBAVersionableInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBAVersionableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 获取当前版本号
	 * @return 版本号
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Version")
	int32 GetVersion() const;
	virtual int32 GetVersion_Implementation() const { return 1; }

	/**
	 * 从旧版本迁移数据
	 * @param OldVersion 旧版本号
	 * @return 是否迁移成功
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Version")
	bool MigrateFromVersion(int32 OldVersion);
	virtual bool MigrateFromVersion_Implementation(int32 OldVersion) { return true; }
};
