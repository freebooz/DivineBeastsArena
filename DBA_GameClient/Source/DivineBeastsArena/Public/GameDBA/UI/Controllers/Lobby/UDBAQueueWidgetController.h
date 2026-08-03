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
#include "GameCore/UI/DBAWidgetController.h"
#include "UDBAQueueWidgetController.generated.h"

/**
 * 队列状态委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueueStateChangedDelegate, int32, NewState);

/**
 * 匹配成功委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchFoundDelegate, const FString&, MatchId);

/**
 * 队列取消委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueueCancelledDelegate, const FString&, Reason);

/**
 * 队列 Widget 控制器
 *
 * 管理匹配队列 UI 的数据绑定和交互
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAQueueWidgetController : public UDBAWidgetController
{
	GENERATED_BODY()

public:
	UDBAQueueWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestJoinQueue(int32 Mode);

	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestLeaveQueue();

	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestAcceptMatch();

	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestDeclineMatch();

public:
	UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
	FOnQueueStateChangedDelegate OnQueueStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
	FOnMatchFoundDelegate OnMatchFound;

	UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
	FOnQueueCancelledDelegate OnQueueCancelled;
};
