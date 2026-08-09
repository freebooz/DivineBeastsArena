// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/ServerDirectory/DBAServerSelectScreenBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/SafeZone.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "GameDBA/Frontend/ServerDirectory/DBAServerListItemWidgetBase.h"
#include "GameDBA/Frontend/ServerDirectory/DBAServerSelectViewModel.h"
#include "GameDBA/Frontend/ServerDirectory/DBAServerSelectWidgetController.h"

#define LOCTEXT_NAMESPACE "DBAServerSelectScreen"

namespace
{
	UButton* CreateTextButton(UWidgetTree* WidgetTree, const FName Name, const FText& Label)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("%sText"), *Name.ToString()));
		Text->SetText(Label);
		Button->SetContent(Text);
		return Button;
	}
}

void UDBAServerSelectScreenBase::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureFallbackLayout();
	if (!WidgetController)
	{
		WidgetController = NewObject<UDBAServerSelectWidgetController>(this);
	}

	OnBackRequested.RemoveDynamic(this, &UDBAServerSelectScreenBase::HandleBackRequested);
	OnBackRequested.AddDynamic(this, &UDBAServerSelectScreenBase::HandleBackRequested);
	RefreshButton->OnClicked.RemoveDynamic(this, &UDBAServerSelectScreenBase::HandleRefreshClicked);
	RefreshButton->OnClicked.AddDynamic(this, &UDBAServerSelectScreenBase::HandleRefreshClicked);
	RetryButton->OnClicked.RemoveDynamic(this, &UDBAServerSelectScreenBase::HandleRetryClicked);
	RetryButton->OnClicked.AddDynamic(this, &UDBAServerSelectScreenBase::HandleRetryClicked);
	ConfirmButton->OnClicked.RemoveDynamic(this, &UDBAServerSelectScreenBase::HandleConfirmClicked);
	ConfirmButton->OnClicked.AddDynamic(this, &UDBAServerSelectScreenBase::HandleConfirmClicked);
	ActivateScreen();
}

void UDBAServerSelectScreenBase::NativeDestruct()
{
	if (UDBAServerSelectViewModel* ViewModel = GetViewModel())
	{
		ViewModel->OnChanged.RemoveDynamic(this, &UDBAServerSelectScreenBase::HandleViewModelChanged);
	}
	Super::NativeDestruct();
}

void UDBAServerSelectScreenBase::ActivateScreen()
{
	if (!WidgetController)
	{
		return;
	}

	WidgetController->Start();
	if (UDBAServerSelectViewModel* ViewModel = GetViewModel())
	{
		ViewModel->OnChanged.RemoveDynamic(this, &UDBAServerSelectScreenBase::HandleViewModelChanged);
		ViewModel->OnChanged.AddDynamic(this, &UDBAServerSelectScreenBase::HandleViewModelChanged);
	}
	RefreshPresentation();
}

UDBAServerSelectViewModel* UDBAServerSelectScreenBase::GetViewModel() const
{
	return WidgetController ? WidgetController->GetViewModel() : nullptr;
}

void UDBAServerSelectScreenBase::EnsureFallbackLayout()
{
	if (!WidgetTree || ServerList)
	{
		return;
	}

	USafeZone* SafeZone = WidgetTree->ConstructWidget<USafeZone>(USafeZone::StaticClass(), TEXT("SafeZone"));
	UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ServerSelectLayout"));
	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	ServerList = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("ServerList"));
	RefreshButton = CreateTextButton(WidgetTree, TEXT("RefreshButton"), LOCTEXT("Refresh", "刷新"));
	RetryButton = CreateTextButton(WidgetTree, TEXT("RetryButton"), LOCTEXT("Retry", "重试"));
	ConfirmButton = CreateTextButton(WidgetTree, TEXT("ConfirmButton"), LOCTEXT("Confirm", "进入区服"));
	TitleText->SetText(LOCTEXT("Title", "选择服务器"));
	Layout->AddChildToVerticalBox(TitleText);
	Layout->AddChildToVerticalBox(StatusText);
	Layout->AddChildToVerticalBox(ServerList);
	Layout->AddChildToVerticalBox(RefreshButton);
	Layout->AddChildToVerticalBox(RetryButton);
	Layout->AddChildToVerticalBox(ConfirmButton);
	SafeZone->AddChild(Layout);
	WidgetTree->RootWidget = SafeZone;
}

void UDBAServerSelectScreenBase::RefreshPresentation()
{
	UDBAServerSelectViewModel* ViewModel = GetViewModel();
	if (!ViewModel || !ServerList)
	{
		return;
	}

	ServerList->ClearChildren();
	TSubclassOf<UDBAServerListItemWidgetBase> ItemClass = ServerListItemWidgetClass;
	if (!ItemClass)
	{
		ItemClass = UDBAServerListItemWidgetBase::StaticClass();
	}
	for (const FDBAServerSelectItemViewData& Entry : ViewModel->GetServers())
	{
		UDBAServerListItemWidgetBase* Item = CreateWidget<UDBAServerListItemWidgetBase>(GetOwningPlayer(), ItemClass);
		if (!Item)
		{
			continue;
		}
		Item->SetViewData(Entry);
		Item->OnChosen.AddDynamic(this, &UDBAServerSelectScreenBase::HandleServerChosen);
		ServerList->AddChild(Item);
	}

	if (StatusText)
	{
		if (ViewModel->GetOperationState() == EDBAAsyncOperationState::InProgress)
		{
			StatusText->SetText(LOCTEXT("Loading", "正在获取区服目录…"));
		}
		else if (ViewModel->GetLastError().IsError())
		{
			StatusText->SetText(ViewModel->GetLastError().UserMessage);
		}
		else if (ViewModel->IsEmpty())
		{
			StatusText->SetText(LOCTEXT("Empty", "当前没有可展示的区服，请刷新后重试。"));
		}
		else
		{
			StatusText->SetText(FText::GetEmpty());
		}
	}

	RefreshButton->SetIsEnabled(ViewModel->CanRefresh());
	RetryButton->SetIsEnabled(ViewModel->CanRefresh());
	RetryButton->SetVisibility(ViewModel->GetLastError().IsError() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ConfirmButton->SetIsEnabled(ViewModel->CanConfirmSelection());
}

void UDBAServerSelectScreenBase::HandleViewModelChanged()
{
	RefreshPresentation();
}

void UDBAServerSelectScreenBase::HandleRefreshClicked()
{
	if (WidgetController)
	{
		WidgetController->Refresh();
	}
}

void UDBAServerSelectScreenBase::HandleRetryClicked()
{
	if (WidgetController)
	{
		WidgetController->Retry();
	}
}

void UDBAServerSelectScreenBase::HandleConfirmClicked()
{
	if (WidgetController)
	{
		WidgetController->ConfirmSelection();
	}
}

void UDBAServerSelectScreenBase::HandleServerChosen(const FString& ServerId)
{
	if (WidgetController)
	{
		WidgetController->SelectServer(ServerId);
	}
}

void UDBAServerSelectScreenBase::HandleBackRequested()
{
	if (WidgetController)
	{
		WidgetController->RequestBackToLogin();
	}
}

#undef LOCTEXT_NAMESPACE
