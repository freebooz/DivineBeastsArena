// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Widgets/Lobby/ElementSelect/UDBAElementSelectWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/ElementSelect/UDBAElementInfoPanelWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/ElementSelect/UDBAFixedSkillGroupPreviewWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/ElementSelect/UDBAElementSelectWidgetController.h"

UDBAElementSelectWidgetBase::UDBAElementSelectWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SelectedZodiac(EDBAZodiac::None)
	, CurrentSelectedElement(EDBAElement::None)
	, bHasSelectedElement(false)
{
}

void UDBAElementSelectWidgetBase::NativeConstruct()
{
	// UDBAWidgetBase does not have NativeConstruct/NativeDestruct/NativeOnActivated/NativeOnDeactivated
	// The base class is UObject, not UUserWidget
}

void UDBAElementSelectWidgetBase::NativeDestruct()
{
}

void UDBAElementSelectWidgetBase::NativeOnActivated()
{
	RefreshElementList();
}

void UDBAElementSelectWidgetBase::NativeOnDeactivated()
{
	CurrentSelectedElement = EDBAElement::None;
	bHasSelectedElement = false;
}

void UDBAElementSelectWidgetBase::SetWidgetController(UDBAElementSelectWidgetController* InController)
{
	WidgetController = InController;
}

void UDBAElementSelectWidgetBase::SetSelectedZodiac(EDBAZodiac Zodiac)
{
	SelectedZodiac = Zodiac;
}

void UDBAElementSelectWidgetBase::RefreshElementList()
{
	TArray<EDBAElement> AvailableElements;
	AvailableElements.Add(EDBAElement::Gold);
	AvailableElements.Add(EDBAElement::Wood);
	AvailableElements.Add(EDBAElement::Water);
	AvailableElements.Add(EDBAElement::Fire);
	AvailableElements.Add(EDBAElement::Earth);

	BP_OnRefreshElementList(AvailableElements);
}

void UDBAElementSelectWidgetBase::SelectElement(EDBAElement Element)
{
	if (Element == EDBAElement::None)
	{
		return;
	}

	CurrentSelectedElement = Element;
	bHasSelectedElement = true;

	BP_OnElementSelected(Element);
	BP_OnConfirmButtonStateChanged(true);

	if (ElementInfoPanel)
	{
		ElementInfoPanel->SetElement(Element);
	}

	if (SkillGroupPreview)
	{
		SkillGroupPreview->SetZodiacAndElement(SelectedZodiac, Element);
	}
}

void UDBAElementSelectWidgetBase::ConfirmElementSelection()
{
	if (!bHasSelectedElement || !WidgetController)
	{
		return;
	}

	WidgetController->ConfirmElementSelection(CurrentSelectedElement);
}

void UDBAElementSelectWidgetBase::OnBackButtonClicked()
{
	if (WidgetController)
	{
		WidgetController->RequestBack();
	}
}

