// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Lobby/DBALobbyMonsterHealthBarWidget.h"

#include "SlateOptMacros.h"
#include "Widgets/Notifications/SProgressBar.h"

TSharedRef<SWidget> UDBALobbyMonsterHealthBarWidget::RebuildWidget()
{
	HealthProgressBar = SNew(SProgressBar)
		.Percent(CachedPercent)
		.FillColorAndOpacity(ResolveFillColor());
	return HealthProgressBar.ToSharedRef();
}

void UDBALobbyMonsterHealthBarWidget::SetHealthPercent(float Percent)
{
	CachedPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
	if (HealthProgressBar.IsValid())
	{
		HealthProgressBar->SetPercent(CachedPercent);
		HealthProgressBar->SetFillColorAndOpacity(ResolveFillColor());
	}
}

void UDBALobbyMonsterHealthBarWidget::SetSelected(bool bSelected)
{
	bCachedSelected = bSelected;
	if (HealthProgressBar.IsValid())
	{
		HealthProgressBar->SetFillColorAndOpacity(ResolveFillColor());
	}
}

FLinearColor UDBALobbyMonsterHealthBarWidget::ResolveFillColor() const
{
	return bCachedSelected
		? FLinearColor(1.0f, 0.02f, 0.0f, 1.0f)
		: FLinearColor(0.92f, 0.0f, 0.0f, 1.0f);
}
