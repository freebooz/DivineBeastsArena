// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Spectator/UI/DBASpectatorStatusBarWidgetBase.h"
#include "Components/Image.h"
#include "Components/Widget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

UDBASpectatorStatusBarWidgetBase::UDBASpectatorStatusBarWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MaxMemberCount(5)
{
}

void UDBASpectatorStatusBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// 鍒濆鍖栨垚鍛楿I鏁扮粍
	MemberUIs.Reset();
	MemberUIs.SetNum(MaxMemberCount);
}

void UDBASpectatorStatusBarWidgetBase::UpdateStatus(const TArray<FDBAObserverViewTarget>& TeamMembers, int32 CurrentIndex)
{
		const int32 UpdateCount = FMath::Min(TeamMembers.Num(), MaxMemberCount);
	for (int32 i = 0; i < UpdateCount; ++i)
	{
		UpdateMemberStatus(i, TeamMembers[i]);
	}

	// 闅愯棌澶氫綑鐨勬垚鍛楿I
	for (int32 i = TeamMembers.Num(); i < MaxMemberCount; ++i)
	{
		if (MemberUIs[i].Container)
		{
			MemberUIs[i].Container->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UDBASpectatorStatusBarWidgetBase::UpdateMemberStatus(int32 Index, const FDBAObserverViewTarget& Member)
{
		if (Index < 0 || Index >= MemberUIs.Num())
	{
		return;
	}

	FDBASpectatorMemberUI& MemberUI = MemberUIs[Index];

	// 鏇存柊鍚嶅瓧
	if (MemberUI.NameText)
	{
		MemberUI.NameText->SetText(FText::FromName(Member.PlayerName));
	}

		if (MemberUI.HPBar)
	{
		MemberUI.HPBar->SetPercent(Member.GetHPPercent());
	}

		if (MemberUI.EnergyBar)
	{
		MemberUI.EnergyBar->SetPercent(Member.GetEnergyPercent());
	}

		if (MemberUI.ActiveIndicator)
	{
		// Member.IsCurrentTarget 鏉ヨ嚜 FDBAObserverViewTarget
				MemberUI.ActiveIndicator->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// 鏄剧ず瀹瑰櫒
	if (MemberUI.Container)
	{
		MemberUI.Container->SetVisibility(ESlateVisibility::Visible);
	}
}

