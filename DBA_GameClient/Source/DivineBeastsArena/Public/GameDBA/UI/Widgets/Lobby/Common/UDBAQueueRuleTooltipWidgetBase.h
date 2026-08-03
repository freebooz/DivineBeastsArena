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
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBAQueueRuleTooltipWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAQueueRuleTooltipWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAQueueRuleTooltipWidgetBase(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|QueueRule")
	void ShowQueueRule(
		const FText& QueueName,
		int32 TeamSize,
		const FText& MapName,
		const FText& ModeName,
		const FText& RuleDescription
	);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|QueueRule", meta = (DisplayName = "On Update Queue Rule"))
	void BP_OnUpdateQueueRule(
		const FText& QueueName,
		int32 TeamSize,
		const FText& MapName,
		const FText& ModeName,
		const FText& RuleDescription
	);
};
