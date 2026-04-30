// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/DBAQueueModeSelectWidgetBase.h"

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
	QuickMatch.ModeName = FText::FromString(TEXT("快速匹配"));
	QuickMatch.ModeDescription = FText::FromString(TEXT("5v5 经典模式，快速匹配"));
	QuickMatch.MapName = FText::FromString(TEXT("神兽竞技场"));
	QuickMatch.EstimatedWaitTime = FText::FromString(TEXT("2-5 分钟"));
	QuickMatch.bIsAvailable = true;
	ModeList.Add(QuickMatch);

	FDBAQueueModeSelectData Ranked;
	Ranked.Mode = EDBAQueueModeSelectMode::Ranked;
	Ranked.ModeName = FText::FromString(TEXT("排位赛"));
	Ranked.ModeDescription = FText::FromString(TEXT("5v5 排位赛，严格匹配"));
	Ranked.MapName = FText::FromString(TEXT("神兽竞技场"));
	Ranked.EstimatedWaitTime = FText::FromString(TEXT("5-10 分钟"));
	Ranked.bIsAvailable = false;
	Ranked.UnavailableReason = FText::FromString(TEXT("需要等级 10 以上"));
	ModeList.Add(Ranked);

	FDBAQueueModeSelectData Practice;
	Practice.Mode = EDBAQueueModeSelectMode::Practice;
	Practice.ModeName = FText::FromString(TEXT("练习模式"));
	Practice.ModeDescription = FText::FromString(TEXT("单人练习，AI 对手"));
	Practice.MapName = FText::FromString(TEXT("训练场"));
	Practice.EstimatedWaitTime = FText::FromString(TEXT("立即开始"));
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
}

void UDBAQueueModeSelectWidgetBase::CancelSelect()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
