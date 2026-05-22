// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Lobby/ElementSelect/UDBAElementInfoPanelWidgetBase.h"
#include "GameCore/Types/DBACommonEnums.h"

UDBAElementInfoPanelWidgetBase::UDBAElementInfoPanelWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentElement(EDBAElement::None)
{
}

void UDBAElementInfoPanelWidgetBase::SetElement(EDBAElement Element)
{
	CurrentElement = Element;

	FText ElementName = FText::GetEmpty();
	FText ElementDescription = FText::FromString(TEXT("閲戝睘鎬э紝閿嬪埄鍧氬浐锛屾搮闀跨墿鐞嗘敾鍑诲拰闃插尽"));

	EDBAElement CounterTo = EDBAElement::Wood;
	EDBAElement CounteredBy = EDBAElement::Fire;

	BP_OnUpdateElementInfo(Element, ElementName, ElementDescription, CounterTo, CounteredBy);
}

