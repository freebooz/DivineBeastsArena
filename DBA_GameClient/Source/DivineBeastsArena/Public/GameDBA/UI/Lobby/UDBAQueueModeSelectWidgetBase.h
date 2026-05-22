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
#include "UDBAQueueModeSelectWidgetBase.generated.h"

UENUM(BlueprintType)
enum class EDBAQueueModeSelectMode : uint8
{
	QuickMatch UMETA(DisplayName = "快速匹配"),
	Ranked UMETA(DisplayName = "排位赛"),
	Custom UMETA(DisplayName = "自定义"),
	Practice UMETA(DisplayName = "练习模式")
};

USTRUCT(BlueprintType)
struct FDBAQueueModeSelectData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	EDBAQueueModeSelectMode Mode;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	FText ModeName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	FText ModeDescription;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	FText MapName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	FText EstimatedWaitTime;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	bool bIsAvailable;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	FText UnavailableReason;

	FDBAQueueModeSelectData()
		: Mode(EDBAQueueModeSelectMode::QuickMatch)
		, bIsAvailable(true)
	{
	}
};

/**
 * Queue mode selection widget base.
 * C++ owns selection/start/cancel behavior; Blueprint owns layout and art.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAQueueModeSelectWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAQueueModeSelectWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|QueueModeSelect")
	void RefreshModeList();

	UFUNCTION(BlueprintCallable, Category = "DBA|QueueModeSelect")
	void SelectMode(EDBAQueueModeSelectMode Mode);

	UFUNCTION(BlueprintCallable, Category = "DBA|QueueModeSelect")
	void StartQueue();

	UFUNCTION(BlueprintCallable, Category = "DBA|QueueModeSelect")
	void CancelSelect();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|QueueModeSelect", meta = (DisplayName = "On Mode List Refreshed"))
	void BP_OnModeListRefreshed(const TArray<FDBAQueueModeSelectData>& Modes);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|QueueModeSelect", meta = (DisplayName = "On Mode Selection Changed"))
	void BP_OnModeSelectionChanged(EDBAQueueModeSelectMode NewMode);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueModeSelect")
	TArray<FDBAQueueModeSelectData> ModeList;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueModeSelect")
	EDBAQueueModeSelectMode SelectedMode;
};
