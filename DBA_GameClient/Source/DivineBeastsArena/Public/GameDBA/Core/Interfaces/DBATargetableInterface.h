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
#include "DBATargetableInterface.generated.h"

/**
 * 基础目标代理接口
 * 用于标识 Actor 是否可被选中、锁定、攻击
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBATargetableInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBATargetableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 是否可被选中
	 * @return 是否可被选中
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Targeting")
	bool IsTargetable() const;
	virtual bool IsTargetable_Implementation() const { return true; }

	/**
	 * 获取目标优先级（用于自动选择目标）
	 * @return 优先级，数值越大优先级越高
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Targeting")
	float GetTargetPriority() const;
	virtual float GetTargetPriority_Implementation() const { return 1.0f; }

	/**
	 * 获取目标显示名称
	 * @return 显示名称
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Targeting")
	FText GetTargetDisplayName() const;
	virtual FText GetTargetDisplayName_Implementation() const { return FText::FromString(TEXT("目标")); }
};
