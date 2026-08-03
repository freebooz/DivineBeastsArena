// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Widgets/Lobby/FiveCampSelect/UDBAFiveCampInfoPanelWidgetBase.h"
#include "GameCore/Types/DBACommonEnums.h"

UDBAFiveCampInfoPanelWidgetBase::UDBAFiveCampInfoPanelWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentFiveCamp(EDBAFiveCamp::None)
	, CurrentZodiac(EDBAZodiac::None)
	, CurrentElement(EDBAElement::None)
{
}

void UDBAFiveCampInfoPanelWidgetBase::SetFiveCamp(EDBAFiveCamp FiveCamp, EDBAZodiac Zodiac, EDBAElement Element)
{
	CurrentFiveCamp = FiveCamp;
	CurrentZodiac = Zodiac;
	CurrentElement = Element;

	FText FiveCampName = FText::GetEmpty();
	FText FiveCampDescription = FText::GetEmpty();
	FText AppearanceTheme = FText::GetEmpty();
	FText EffectTheme = FText::GetEmpty();

	BP_OnUpdateFiveCampInfo(FiveCamp, FiveCampName, FiveCampDescription, AppearanceTheme, EffectTheme);
}

