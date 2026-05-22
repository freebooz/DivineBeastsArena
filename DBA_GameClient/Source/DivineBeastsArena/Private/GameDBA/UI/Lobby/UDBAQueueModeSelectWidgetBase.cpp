// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Lobby/UDBAQueueModeSelectWidgetBase.h"

#include "GameDBA/UI/DBAGameUIManager.h"

UDBAQueueModeSelectWidgetBase::UDBAQueueModeSelectWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SelectedMode(EDBAQueueModeSelectMode::QuickMatch)
{
}

void UDBAQueueModeSelectWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBAQueueModeSelectWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshModeList();
}

void UDBAQueueModeSelectWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAQueueModeSelectWidgetBase::RefreshModeList()
{
	ModeList.Empty();

	FDBAQueueModeSelectData QuickMatch;
	QuickMatch.Mode = EDBAQueueModeSelectMode::QuickMatch;
	QuickMatch.ModeName = NSLOCTEXT("DBAQueueModeSelect", "QuickMatchName", "快速匹配");
	QuickMatch.ModeDescription = NSLOCTEXT("DBAQueueModeSelect", "QuickMatchDesc", "标准 5v5 对战，自动匹配队伍。");
	QuickMatch.MapName = NSLOCTEXT("DBAQueueModeSelect", "DefaultArenaMap", "五行竞技场");
	QuickMatch.EstimatedWaitTime = NSLOCTEXT("DBAQueueModeSelect", "QuickMatchEstimate", "约 2-5 分钟");
	QuickMatch.bIsAvailable = true;
	ModeList.Add(QuickMatch);

	FDBAQueueModeSelectData Ranked;
	Ranked.Mode = EDBAQueueModeSelectMode::Ranked;
	Ranked.ModeName = NSLOCTEXT("DBAQueueModeSelect", "RankedName", "排位赛");
	Ranked.ModeDescription = NSLOCTEXT("DBAQueueModeSelect", "RankedDesc", "5v5 排位赛，使用更严格的实力匹配。");
	Ranked.MapName = NSLOCTEXT("DBAQueueModeSelect", "RankedMap", "五行竞技场");
	Ranked.EstimatedWaitTime = NSLOCTEXT("DBAQueueModeSelect", "RankedEstimate", "约 5-10 分钟");
	Ranked.bIsAvailable = false;
	Ranked.UnavailableReason = NSLOCTEXT("DBAQueueModeSelect", "RankedUnavailable", "需要等级 10 以上");
	ModeList.Add(Ranked);

	FDBAQueueModeSelectData Practice;
	Practice.Mode = EDBAQueueModeSelectMode::Practice;
	Practice.ModeName = NSLOCTEXT("DBAQueueModeSelect", "PracticeName", "练习模式");
	Practice.ModeDescription = NSLOCTEXT("DBAQueueModeSelect", "PracticeDesc", "单人练习，AI 对手。");
	Practice.MapName = NSLOCTEXT("DBAQueueModeSelect", "PracticeMap", "训练场");
	Practice.EstimatedWaitTime = NSLOCTEXT("DBAQueueModeSelect", "PracticeEstimate", "立即开始");
	Practice.bIsAvailable = true;
	ModeList.Add(Practice);

	BP_OnModeListRefreshed(ModeList);
}

void UDBAQueueModeSelectWidgetBase::SelectMode(EDBAQueueModeSelectMode Mode)
{
	SelectedMode = Mode;
	BP_OnModeSelectionChanged(Mode);
}

void UDBAQueueModeSelectWidgetBase::StartQueue()
{
	const FDBAQueueModeSelectData* FoundMode = ModeList.FindByPredicate([this](const FDBAQueueModeSelectData& Mode)
	{
		return Mode.Mode == SelectedMode;
	});

	if (FoundMode && !FoundMode->bIsAvailable)
	{
		return;
	}
	if (!FoundMode && ModeList.Num() == 0)
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			const FDBAQueueModeSelectData& QueueMode = FoundMode ? *FoundMode : ModeList[0];
			UIManager->HideQueueModeSelect();
			UIManager->ShowQueueStatus(QueueMode.ModeName, QueueMode.MapName, QueueMode.EstimatedWaitTime);
		}
	}
}

void UDBAQueueModeSelectWidgetBase::CancelSelect()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->HideQueueModeSelect();
			return;
		}
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

