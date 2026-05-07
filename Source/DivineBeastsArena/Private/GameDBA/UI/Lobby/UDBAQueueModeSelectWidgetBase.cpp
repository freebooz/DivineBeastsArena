// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/UDBAQueueModeSelectWidgetBase.h"

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
	QuickMatch.ModeName = FText::GetEmpty();
	QuickMatch.ModeDescription = FText::GetEmpty();
	QuickMatch.MapName = FText::GetEmpty();
	QuickMatch.EstimatedWaitTime = FText::FromString(TEXT("2-5 鍒嗛挓"));
	QuickMatch.bIsAvailable = true;
	ModeList.Add(QuickMatch);

	FDBAQueueModeSelectData Ranked;
	Ranked.Mode = EDBAQueueModeSelectMode::Ranked;
	Ranked.ModeName = FText::GetEmpty();
	Ranked.ModeDescription = FText::FromString(TEXT("5v5 鎺掍綅璧涳紝涓ユ牸鍖归厤"));
	Ranked.MapName = FText::GetEmpty();
	Ranked.EstimatedWaitTime = FText::FromString(TEXT("5-10 鍒嗛挓"));
	Ranked.bIsAvailable = false;
	Ranked.UnavailableReason = FText::FromString(TEXT("闇€瑕佺瓑绾?10 浠ヤ笂"));
	ModeList.Add(Ranked);

	FDBAQueueModeSelectData Practice;
	Practice.Mode = EDBAQueueModeSelectMode::Practice;
	Practice.ModeName = FText::FromString(TEXT("缁冧範妯″紡"));
	Practice.ModeDescription = FText::FromString(TEXT("鍗曚汉缁冧範锛孉I 瀵规墜"));
	Practice.MapName = FText::GetEmpty();
	Practice.EstimatedWaitTime = FText::GetEmpty();
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

