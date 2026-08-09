// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/ServerDirectory/DBAServerListItemWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "DBAServerListItem"

void UDBAServerListItemWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureFallbackLayout();
	SelectButton->OnClicked.RemoveDynamic(this, &UDBAServerListItemWidgetBase::HandleClicked);
	SelectButton->OnClicked.AddDynamic(this, &UDBAServerListItemWidgetBase::HandleClicked);
	RefreshPresentation();
}

void UDBAServerListItemWidgetBase::SetViewData(const FDBAServerSelectItemViewData& InViewData)
{
	ViewData = InViewData;
	RefreshPresentation();
}

void UDBAServerListItemWidgetBase::EnsureFallbackLayout()
{
	if (!WidgetTree || SelectButton)
	{
		return;
	}

	SelectButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SelectButton"));
	SummaryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SummaryText"));
	SelectButton->SetContent(SummaryText);
	WidgetTree->RootWidget = SelectButton;
}

void UDBAServerListItemWidgetBase::RefreshPresentation()
{
	if (!SummaryText)
	{
		return;
	}

	FText Tags;
	if (ViewData.bRecommended && ViewData.bIsLastLoginServer)
	{
		Tags = LOCTEXT("RecommendedAndLast", "推荐 · 上次登录");
	}
	else if (ViewData.bRecommended)
	{
		Tags = LOCTEXT("Recommended", "推荐");
	}
	else if (ViewData.bIsLastLoginServer)
	{
		Tags = LOCTEXT("LastLogin", "上次登录");
	}

	SummaryText->SetText(FText::Format(
		LOCTEXT("Summary", "{0}\n{1} · {2} · {3}\n{4}{5}"),
		ViewData.Name,
		ViewData.RegionText,
		ViewData.StatusText,
		ViewData.PopulationText,
		Tags,
		ViewData.UnavailableReason.IsEmpty() ? FText::GetEmpty() : FText::Format(LOCTEXT("Reason", "\n{0}"), ViewData.UnavailableReason)));

	if (SelectButton)
	{
		SelectButton->SetIsEnabled(ViewData.bCanSelect);
	}
}

void UDBAServerListItemWidgetBase::HandleClicked()
{
	if (ViewData.bCanSelect && !ViewData.ServerId.IsEmpty())
	{
		OnChosen.Broadcast(ViewData.ServerId);
	}
}

#undef LOCTEXT_NAMESPACE
