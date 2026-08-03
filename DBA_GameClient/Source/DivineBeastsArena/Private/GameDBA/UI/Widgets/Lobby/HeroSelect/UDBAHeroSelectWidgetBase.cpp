// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Widgets/Lobby/HeroSelect/UDBAHeroSelectWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/HeroSelect/UDBAHeroInfoPanelWidgetBase.h"
#include "GameDBA/UI/Widgets/Lobby/HeroSelect/UDBAHeroSelectWidgetController.h"

UDBAHeroSelectWidgetBase::UDBAHeroSelectWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentSelectedZodiac(EDBAZodiac::None)
	, bHasSelectedZodiac(false)
{
}

void UDBAHeroSelectWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBAHeroSelectWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAHeroSelectWidgetBase::NativeOnActivated()
{
	RefreshZodiacList();
}

void UDBAHeroSelectWidgetBase::NativeOnDeactivated()
{
	CurrentSelectedZodiac = EDBAZodiac::None;
	bHasSelectedZodiac = false;
}

void UDBAHeroSelectWidgetBase::SetWidgetController(UDBAHeroSelectWidgetController* InController)
{
	WidgetController = InController;
}

void UDBAHeroSelectWidgetBase::RefreshZodiacList()
{
	TArray<EDBAZodiac> AvailableZodiacs;
	AvailableZodiacs.Add(EDBAZodiac::Rat);
	AvailableZodiacs.Add(EDBAZodiac::Ox);
	AvailableZodiacs.Add(EDBAZodiac::Tiger);
	AvailableZodiacs.Add(EDBAZodiac::Rabbit);
	AvailableZodiacs.Add(EDBAZodiac::Dragon);
	AvailableZodiacs.Add(EDBAZodiac::Snake);
	AvailableZodiacs.Add(EDBAZodiac::Horse);
	AvailableZodiacs.Add(EDBAZodiac::Goat);
	AvailableZodiacs.Add(EDBAZodiac::Monkey);
	AvailableZodiacs.Add(EDBAZodiac::Rooster);
	AvailableZodiacs.Add(EDBAZodiac::Dog);
	AvailableZodiacs.Add(EDBAZodiac::Pig);

	BP_OnRefreshZodiacList(AvailableZodiacs);
}

void UDBAHeroSelectWidgetBase::SelectZodiac(EDBAZodiac Zodiac)
{
	if (Zodiac == EDBAZodiac::None)
	{
		return;
	}

	CurrentSelectedZodiac = Zodiac;
	bHasSelectedZodiac = true;

	BP_OnZodiacSelected(Zodiac);
	BP_OnConfirmButtonStateChanged(true);

	if (HeroInfoPanel)
	{
		HeroInfoPanel->SetZodiac(Zodiac);
	}
}

void UDBAHeroSelectWidgetBase::ConfirmZodiacSelection()
{
	if (!bHasSelectedZodiac || !WidgetController)
	{
		return;
	}

	WidgetController->ConfirmZodiacSelection(CurrentSelectedZodiac);
}

void UDBAHeroSelectWidgetBase::OnBackButtonClicked()
{
	if (WidgetController)
	{
		WidgetController->RequestBack();
	}
}

