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
#include "UDBAElementSelectWidgetBase.generated.h"

class UDBAElementInfoPanelWidgetBase;
class UDBAFixedSkillGroupPreviewWidgetBase;
class UDBAElementSelectWidgetController;

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAElementSelectWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAElementSelectWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	virtual void NativeOnActivated();
	virtual void NativeOnDeactivated();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void SetWidgetController(UDBAElementSelectWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void SetSelectedZodiac(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void RefreshElementList();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void SelectElement(EDBAElement Element);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void ConfirmElementSelection();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ElementSelect")
	void OnBackButtonClicked();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ElementSelect", meta = (DisplayName = "On Refresh Element List"))
	void BP_OnRefreshElementList(const TArray<EDBAElement>& AvailableElements);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ElementSelect", meta = (DisplayName = "On Element Selected"))
	void BP_OnElementSelected(EDBAElement Element);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ElementSelect", meta = (DisplayName = "On Confirm Button State Changed"))
	void BP_OnConfirmButtonStateChanged(bool bCanConfirm);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect", meta = (BindWidgetOptional))
	TObjectPtr<UDBAElementInfoPanelWidgetBase> ElementInfoPanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect", meta = (BindWidgetOptional))
	TObjectPtr<UDBAFixedSkillGroupPreviewWidgetBase> SkillGroupPreview;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect")
	TObjectPtr<UDBAElementSelectWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect")
	EDBAZodiac SelectedZodiac;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect")
	EDBAElement CurrentSelectedElement;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ElementSelect")
	bool bHasSelectedElement;
};
