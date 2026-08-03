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
#include "UDBAPortalConfirmWidgetBase.generated.h"

/**
 * DBAPortalConfirmWidgetBase
 *
 * 传送门确认 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAPortalConfirmWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAPortalConfirmWidgetBase(const FObjectInitializer& ObjectInitializer);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPortalConfirmed, FName, DestinationId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPortalCancelled);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|PortalConfirm")
	void ShowConfirm(FName DestinationId, const FText& DestinationName, const FText& DestinationDescription, bool bCanTeleport, const FText& ConditionText);

	UFUNCTION(BlueprintCallable, Category = "DBA|PortalConfirm")
	void ConfirmTeleport();

	UFUNCTION(BlueprintCallable, Category = "DBA|PortalConfirm")
	void CancelTeleport();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|PortalConfirm", meta = (DisplayName = "On Show Confirm"))
	void BP_OnShowConfirm(const FText& DestinationName, const FText& DestinationDescription, bool bCanTeleport, const FText& ConditionText);

public:
	UPROPERTY(BlueprintAssignable, Category = "DBA|PortalConfirm")
	FOnPortalConfirmed OnPortalConfirmedEvent;

	UPROPERTY(BlueprintAssignable, Category = "DBA|PortalConfirm")
	FOnPortalCancelled OnPortalCancelledEvent;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|PortalConfirm")
	FName DestinationId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|PortalConfirm")
	FText CachedDestinationName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|PortalConfirm")
	FText CachedDestinationDescription;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|PortalConfirm")
	bool CachedCanTeleport;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|PortalConfirm")
	FText CachedConditionText;
};
