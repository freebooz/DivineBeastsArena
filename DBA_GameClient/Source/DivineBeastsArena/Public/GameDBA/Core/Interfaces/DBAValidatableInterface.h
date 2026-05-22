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
#include "DBAValidatableInterface.generated.h"

/**
 * 可校验对象接口
 * 用于 DataTable、DataAsset、配置对象的数据完整性校验
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBAValidatableInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBAValidatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 校验数据完整性
	 * @param OutErrors 输出错误列表
	 * @return 是否校验通过
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Validation")
	bool ValidateData(TArray<FString>& OutErrors) const;
	virtual bool ValidateData_Implementation(TArray<FString>& OutErrors) const { return true; }
};
