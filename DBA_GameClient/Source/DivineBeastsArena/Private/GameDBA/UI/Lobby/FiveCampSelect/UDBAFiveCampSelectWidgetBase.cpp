// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Lobby/FiveCampSelect/UDBAFiveCampSelectWidgetBase.h"
#include "GameDBA/UI/Lobby/FiveCampSelect/UDBAFiveCampInfoPanelWidgetBase.h"
#include "GameDBA/UI/Lobby/FiveCampSelect/UDBAFiveCampSelectWidgetController.h"

UDBAFiveCampSelectWidgetBase::UDBAFiveCampSelectWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SelectedZodiac(EDBAZodiac::None)
	, SelectedElement(EDBAElement::None)
	, CurrentSelectedFiveCamp(EDBAFiveCamp::None)
	, bHasSelectedFiveCamp(false)
{
}

void UDBAFiveCampSelectWidgetBase::NativeConstruct()
{
	// UDBAWidgetBase does not have NativeConstruct/NativeDestruct/NativeOnActivated/NativeOnDeactivated
	// The base class is UObject, not UUserWidget
}

void UDBAFiveCampSelectWidgetBase::NativeDestruct()
{
}

void UDBAFiveCampSelectWidgetBase::NativeOnActivated()
{
	RefreshFiveCampList();
}

void UDBAFiveCampSelectWidgetBase::NativeOnDeactivated()
{
	CurrentSelectedFiveCamp = EDBAFiveCamp::None;
	bHasSelectedFiveCamp = false;
}

void UDBAFiveCampSelectWidgetBase::SetWidgetController(UDBAFiveCampSelectWidgetController* InController)
{
	WidgetController = InController;
}

void UDBAFiveCampSelectWidgetBase::SetSelectedZodiacAndElement(EDBAZodiac Zodiac, EDBAElement Element)
{
	SelectedZodiac = Zodiac;
	SelectedElement = Element;
}

void UDBAFiveCampSelectWidgetBase::RefreshFiveCampList()
{
	TArray<EDBAFiveCamp> AvailableFiveCamps;
	AvailableFiveCamps.Add(EDBAFiveCamp::Center);
	AvailableFiveCamps.Add(EDBAFiveCamp::East);
	AvailableFiveCamps.Add(EDBAFiveCamp::West);
	AvailableFiveCamps.Add(EDBAFiveCamp::South);
	AvailableFiveCamps.Add(EDBAFiveCamp::North);

	BP_OnRefreshFiveCampList(AvailableFiveCamps);
}

void UDBAFiveCampSelectWidgetBase::SelectFiveCamp(EDBAFiveCamp FiveCamp)
{
	if (FiveCamp == EDBAFiveCamp::None)
	{
		return;
	}

	CurrentSelectedFiveCamp = FiveCamp;
	bHasSelectedFiveCamp = true;

	BP_OnFiveCampSelected(FiveCamp);
	BP_OnConfirmButtonStateChanged(true);

	if (FiveCampInfoPanel)
	{
		FiveCampInfoPanel->SetFiveCamp(FiveCamp, SelectedZodiac, SelectedElement);
	}
}

void UDBAFiveCampSelectWidgetBase::ConfirmFiveCampSelection()
{
	if (!bHasSelectedFiveCamp || !WidgetController)
	{
		return;
	}

	WidgetController->ConfirmFiveCampSelection(CurrentSelectedFiveCamp);
}

void UDBAFiveCampSelectWidgetBase::OnBackButtonClicked()
{
	if (WidgetController)
	{
		WidgetController->RequestBack();
	}
}

