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
#include "UDBAQueueStatusWidgetBase.generated.h"

/**
 * DBAQueueStatusWidgetBase
 *
 * 匹配状态 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAQueueStatusWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAQueueStatusWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|QueueStatus")
	void StartQueue(const FText& ModeName, const FText& MapName, const FText& EstimatedWaitTime);

	UFUNCTION(BlueprintCallable, Category = "DBA|QueueStatus")
	void CancelQueue();

	UFUNCTION(BlueprintCallable, Category = "DBA|QueueStatus")
	void UpdateWaitTime(float ElapsedTime);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|QueueStatus", meta = (DisplayName = "On Queue Started"))
	void BP_OnQueueStarted(const FText& ModeName, const FText& MapName, const FText& EstimatedWaitTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|QueueStatus", meta = (DisplayName = "On Wait Time Updated"))
	void BP_OnWaitTimeUpdated(const FText& WaitTimeText);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|QueueStatus", meta = (DisplayName = "On Queue Cancelled"))
	void BP_OnQueueCancelled();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueStatus")
	FText CachedModeName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueStatus")
	FText CachedMapName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueStatus")
	FText CachedEstimatedWaitTime;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueStatus")
	float ElapsedWaitTime;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueStatus")
	bool bIsQueuing;
};
