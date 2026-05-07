// Copyright Freebooz Games, Inc. All Rights Reserved.

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

