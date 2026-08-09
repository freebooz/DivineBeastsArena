// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Framework/DBAUIRootLayout.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

TSharedRef<SWidget> UDBAUIRootLayout::RebuildWidget()
{
	EnsureLayerTree();
	return Super::RebuildWidget();
}

void UDBAUIRootLayout::EnsureLayerTree()
{
	if (RootOverlay || !WidgetTree)
	{
		return;
	}

	RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
	WidgetTree->RootWidget = RootOverlay;

	auto AddLayer = [this](const FName Name) -> UOverlay*
	{
		UOverlay* Layer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), Name);
		RootOverlay->AddChildToOverlay(Layer);
		return Layer;
	};

	BackgroundLayer = AddLayer(TEXT("BackgroundLayer"));
	ScreenLayer = AddLayer(TEXT("ScreenLayer"));
	ModalLayer = AddLayer(TEXT("ModalLayer"));
	ToastLayer = AddLayer(TEXT("ToastLayer"));
	TooltipLayer = AddLayer(TEXT("TooltipLayer"));
	DebugLayer = AddLayer(TEXT("DebugLayer"));
}

bool UDBAUIRootLayout::MountSingle(UOverlay* Layer, UUserWidget* Widget)
{
	if (!Layer || !Widget)
	{
		return false;
	}

	if (Widget->GetParent() != Layer)
	{
		Layer->ClearChildren();
		Layer->AddChildToOverlay(Widget);
	}

	Widget->SetVisibility(ESlateVisibility::Visible);
	return true;
}

bool UDBAUIRootLayout::MountStacked(UOverlay* Layer, UUserWidget* Widget)
{
	if (!Layer || !Widget)
	{
		return false;
	}

	if (Widget->GetParent() != Layer)
	{
		Layer->AddChildToOverlay(Widget);
	}

	Widget->SetVisibility(ESlateVisibility::Visible);
	return true;
}

bool UDBAUIRootLayout::MountBackground(UUserWidget* Widget)
{
	EnsureLayerTree();
	return MountSingle(BackgroundLayer, Widget);
}

bool UDBAUIRootLayout::MountScreen(UUserWidget* Widget)
{
	EnsureLayerTree();
	return MountSingle(ScreenLayer, Widget);
}

bool UDBAUIRootLayout::MountModal(UUserWidget* Widget)
{
	EnsureLayerTree();
	return MountStacked(ModalLayer, Widget);
}

bool UDBAUIRootLayout::MountToast(UUserWidget* Widget)
{
	EnsureLayerTree();
	return MountStacked(ToastLayer, Widget);
}

bool UDBAUIRootLayout::MountTooltip(UUserWidget* Widget)
{
	EnsureLayerTree();
	return MountStacked(TooltipLayer, Widget);
}

bool UDBAUIRootLayout::MountDebug(UUserWidget* Widget)
{
	EnsureLayerTree();
	return MountStacked(DebugLayer, Widget);
}

bool UDBAUIRootLayout::RemoveManagedWidget(UUserWidget* Widget)
{
	if (!Widget)
	{
		return false;
	}

	if (Widget->GetParent())
	{
		Widget->RemoveFromParent();
		return true;
	}

	return false;
}

UUserWidget* UDBAUIRootLayout::GetTopModal() const
{
	if (!ModalLayer || ModalLayer->GetChildrenCount() == 0)
	{
		return nullptr;
	}

	return Cast<UUserWidget>(ModalLayer->GetChildAt(ModalLayer->GetChildrenCount() - 1));
}
