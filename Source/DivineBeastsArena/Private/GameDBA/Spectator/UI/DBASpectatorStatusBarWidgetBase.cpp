// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Spectator/UI/DBASpectatorStatusBarWidgetBase.h"

UDBASpectatorStatusBarWidgetBase::UDBASpectatorStatusBarWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MaxMemberCount(5)
{
}

void UDBASpectatorStatusBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化成员UI数组
	MemberUIs.Reset();
	MemberUIs.SetNum(MaxMemberCount);
}

void UDBASpectatorStatusBarWidgetBase::UpdateStatus(const TArray<FDBAObserverViewTarget>& TeamMembers, int32 CurrentIndex)
{
	// 动态更新所有成员状态
	const int32 UpdateCount = FMath::Min(TeamMembers.Num(), MaxMemberCount);
	for (int32 i = 0; i < UpdateCount; ++i)
	{
		UpdateMemberStatus(i, TeamMembers[i]);
	}

	// 隐藏多余的成员UI
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
	// 检查索引有效性
	if (Index < 0 || Index >= MemberUIs.Num())
	{
		return;
	}

	FDBASpectatorMemberUI& MemberUI = MemberUIs[Index];

	// 更新名字
	if (MemberUI.NameText)
	{
		MemberUI.NameText->SetText(FText::FromName(Member.PlayerName));
	}

	// 更新HP条
	if (MemberUI.HPBar)
	{
		MemberUI.HPBar->SetPercent(Member.GetHPPercent());
	}

	// 更新Energy条
	if (MemberUI.EnergyBar)
	{
		MemberUI.EnergyBar->SetPercent(Member.GetEnergyPercent());
	}

	// 更新活动指示器
	if (MemberUI.ActiveIndicator)
	{
		// Member.IsCurrentTarget 来自 FDBAObserverViewTarget
		// 如果有 CurrentTargetIndex 传入，应该在调用前处理
		MemberUI.ActiveIndicator->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// 显示容器
	if (MemberUI.Container)
	{
		MemberUI.Container->SetVisibility(ESlateVisibility::Visible);
	}
}