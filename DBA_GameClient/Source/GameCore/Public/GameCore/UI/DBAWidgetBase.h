// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DBAWidgetBase.generated.h"

/**
 * DBAWidgetBase
 *
 * UI 控件基类
 * 提供通用的 Widget 功能
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMECORE_API UDBAWidgetBase : public UObject
{
	GENERATED_BODY()

public:
	UDBAWidgetBase(const FObjectInitializer& ObjectInitializer);

	/**
	 * 获取 Widget 的初始名字
	 */
	UFUNCTION(BlueprintPure, Category = "DBA|UI")
	FString GetWidgetName() const { return WidgetName; }

protected:
	/** Widget 名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI")
	FString WidgetName;
};
