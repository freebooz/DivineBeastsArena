// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Widgets/Lobby/ElementSelect/UDBAFixedSkillGroupPreviewWidgetBase.h"

UDBAFixedSkillGroupPreviewWidgetBase::UDBAFixedSkillGroupPreviewWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentZodiac(EDBAZodiac::None)
	, CurrentElement(EDBAElement::None)
{
}

void UDBAFixedSkillGroupPreviewWidgetBase::SetZodiacAndElement(EDBAZodiac Zodiac, EDBAElement Element)
{
	CurrentZodiac = Zodiac;
	CurrentElement = Element;

	FText PassiveName = FText::FromString(TEXT("榧犵帇涔嬫晱"));
	FText Skill01Name = FText::GetEmpty();
	FText Skill02Name = FText::FromString(TEXT("閲戠浘鎶や綋"));
	FText Skill03Name = FText::GetEmpty();
	FText Skill04Name = FText::GetEmpty();
	FText UltimateName = FText::FromString(TEXT("榧犵帇闄嶄复"));

	int32 ResonanceLevel = 4;
	FText ResonanceDescription = FText::FromString(TEXT("鎺у埗鏃堕棿 +1.0绉掞紝鎶ょ浘鍊?+20%"));

	BP_OnUpdateSkillGroupPreview(
		Zodiac,
		Element,
		PassiveName,
		Skill01Name,
		Skill02Name,
		Skill03Name,
		Skill04Name,
		UltimateName,
		ResonanceLevel,
		ResonanceDescription
	);
}

