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
#include "DBAExternalServiceDegradableInterface.generated.h"

/**
 * 可外部服务降级对象接口
 * 用于标识对象支持外部服务（Monitoring / GameOps）降级模式
 * 外部服务不可用时，对象必须能够正常工作
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBAExternalServiceDegradableInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBAExternalServiceDegradableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 是否处于降级模式
	 * @return 是否降级
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ExternalService")
	bool IsDegraded() const;
	virtual bool IsDegraded_Implementation() const { return false; }

	/**
	 * 进入降级模式
	 * @param Reason 降级原因
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ExternalService")
	void EnterDegradedMode(const FString& Reason);
	virtual void EnterDegradedMode_Implementation(const FString& Reason) {}

	/**
	 * 退出降级模式
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ExternalService")
	void ExitDegradedMode();
	virtual void ExitDegradedMode_Implementation() {}
};
