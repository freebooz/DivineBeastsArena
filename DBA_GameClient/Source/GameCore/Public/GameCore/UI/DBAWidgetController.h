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
#include "DBAWidgetController.generated.h"

/**
 * DBAWidgetController
 *
 * Widget 控制器基类
 * 负责管理 Widget 的数据和业务逻辑
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMECORE_API UDBAWidgetController : public UObject
{
	GENERATED_BODY()

public:
	UDBAWidgetController(const FObjectInitializer& ObjectInitializer);

	/**
	 * 初始化控制器
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI")
	virtual void InitializeController();

	/**
	 * 重置控制器状态
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI")
	virtual void ResetController();

protected:
	/** 控制器是否已初始化 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI")
	bool bIsInitialized = false;
};
