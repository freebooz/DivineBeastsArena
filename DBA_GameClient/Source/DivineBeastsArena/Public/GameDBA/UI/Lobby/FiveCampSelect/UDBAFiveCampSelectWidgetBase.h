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
#include "GameCore/Types/DBACommonEnums.h"
#include "UDBAFiveCampSelectWidgetBase.generated.h"

class UDBAFiveCampInfoPanelWidgetBase;
class UDBAFiveCampSelectWidgetController;

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAFiveCampSelectWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAFiveCampSelectWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	virtual void NativeOnActivated();
	virtual void NativeOnDeactivated();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void SetWidgetController(UDBAFiveCampSelectWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void SetSelectedZodiacAndElement(EDBAZodiac Zodiac, EDBAElement Element);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void RefreshFiveCampList();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void SelectFiveCamp(EDBAFiveCamp FiveCamp);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void ConfirmFiveCampSelection();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|FiveCampSelect")
	void OnBackButtonClicked();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|FiveCampSelect", meta = (DisplayName = "On Refresh FiveCamp List"))
	void BP_OnRefreshFiveCampList(const TArray<EDBAFiveCamp>& AvailableFiveCamps);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|FiveCampSelect", meta = (DisplayName = "On FiveCamp Selected"))
	void BP_OnFiveCampSelected(EDBAFiveCamp FiveCamp);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|FiveCampSelect", meta = (DisplayName = "On Confirm Button State Changed"))
	void BP_OnConfirmButtonStateChanged(bool bCanConfirm);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect", meta = (BindWidgetOptional))
	TObjectPtr<UDBAFiveCampInfoPanelWidgetBase> FiveCampInfoPanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect")
	TObjectPtr<UDBAFiveCampSelectWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect")
	EDBAZodiac SelectedZodiac;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect")
	EDBAElement SelectedElement;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect")
	EDBAFiveCamp CurrentSelectedFiveCamp;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|FiveCampSelect")
	bool bHasSelectedFiveCamp;
};
