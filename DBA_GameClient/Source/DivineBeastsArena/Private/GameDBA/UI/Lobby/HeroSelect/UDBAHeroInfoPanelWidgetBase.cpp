// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Lobby/HeroSelect/UDBAHeroInfoPanelWidgetBase.h"

UDBAHeroInfoPanelWidgetBase::UDBAHeroInfoPanelWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentZodiac(EDBAZodiac::None)
{
}

void UDBAHeroInfoPanelWidgetBase::SetZodiac(EDBAZodiac Zodiac)
{
	CurrentZodiac = Zodiac;

	FText ZodiacName = FText::FromString(TEXT("瀛愰紶"));
	FText ZodiacDescription = FText::FromString(TEXT("鐏垫椿鏁忔嵎鐨勭敓鑲栵紝鎿呴暱蹇€熺Щ鍔ㄥ拰杩炵画鏀诲嚮"));
	FText UltimateDescription = FText::FromString(TEXT("榧犵帇闄嶄复锛氬彫鍞ら紶缇ゅ鑼冨洿鍐呮晫浜洪€犳垚鎸佺画浼ゅ"));

	BP_OnUpdateZodiacInfo(Zodiac, ZodiacName, ZodiacDescription, UltimateDescription);
}

