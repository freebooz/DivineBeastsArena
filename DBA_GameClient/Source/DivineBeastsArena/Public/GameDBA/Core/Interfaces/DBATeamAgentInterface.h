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
#include "DBATeamAgentInterface.generated.h"

class IDBATeamAgentInterface;

/**
 * 基础团队代理接口
 * 用于标识 Actor 所属团队，支持敌我识别、目标选择
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBATeamAgentInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBATeamAgentInterface
{
	GENERATED_BODY()

public:
	/**
	 * 获取团队 ID
	 * @return 团队 ID，-1 表示中立
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Team")
	int32 GetTeamId() const;
	virtual int32 GetTeamId_Implementation() const { return -1; }

	/**
	 * 设置团队 ID
	 * @param NewTeamId 新团队 ID
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Team")
	void SetTeamId(int32 NewTeamId);
	virtual void SetTeamId_Implementation(int32 NewTeamId) {}

	/**
	 * 判断是否为敌对关系
	 * @param Other 目标对象
	 * @return 是否为敌对
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Team")
	bool IsHostileTo(const TScriptInterface<IDBATeamAgentInterface>& Other) const;
	virtual bool IsHostileTo_Implementation(const TScriptInterface<IDBATeamAgentInterface>& Other) const { return false; }
};
