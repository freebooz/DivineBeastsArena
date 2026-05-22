// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Lobby/UDBAInventoryWidgetBase.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "GameDBA/UI/DBAGameUIManager.h"
#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	bool IsMobileOrCompactInventoryViewport(UUserWidget* Widget, const FVector2D& ViewportSize)
	{
		const FString PlatformName = UGameplayStatics::GetPlatformName();
		const bool bMobilePlatform = PlatformName.Equals(TEXT("Android"), ESearchCase::IgnoreCase)
			|| PlatformName.Equals(TEXT("IOS"), ESearchCase::IgnoreCase)
			|| PlatformName.Equals(TEXT("iOS"), ESearchCase::IgnoreCase);
		const bool bCompactViewport = FMath::Min(ViewportSize.X, ViewportSize.Y) > 0.0f
			&& FMath::Min(ViewportSize.X, ViewportSize.Y) < 720.0f;
		return bMobilePlatform || bCompactViewport || !Widget;
	}

	FVector2D GetInventoryViewportSize(UUserWidget* Widget)
	{
		if (!Widget)
		{
			return FVector2D::ZeroVector;
		}

		FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(Widget);
		if ((ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f) && Widget->GetWorld() && Widget->GetWorld()->GetGameViewport())
		{
			Widget->GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);
		}
		return ViewportSize;
	}

	FVector2D ResolveResponsiveInventoryPanelSize(UUserWidget* Widget, const FVector2D& DesiredSize)
	{
		const FVector2D ViewportSize = GetInventoryViewportSize(Widget);
		if (ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f)
		{
			return DesiredSize;
		}

		const bool bCompact = IsMobileOrCompactInventoryViewport(Widget, ViewportSize);
		const FVector2D Margin = bCompact ? FVector2D(28.0f, 28.0f) : FVector2D(96.0f, 72.0f);
		const FVector2D MinSize(320.0f, 260.0f);
		const FVector2D MaxSize(
			FMath::Max(MinSize.X, ViewportSize.X - Margin.X),
			FMath::Max(MinSize.Y, ViewportSize.Y - Margin.Y));

		return FVector2D(
			FMath::Clamp(DesiredSize.X, MinSize.X, MaxSize.X),
			FMath::Clamp(DesiredSize.Y, MinSize.Y, MaxSize.Y));
	}

	FMargin ResolveResponsiveInventoryPanelPadding(UUserWidget* Widget)
	{
		const FVector2D ViewportSize = GetInventoryViewportSize(Widget);
		const bool bCompact = IsMobileOrCompactInventoryViewport(Widget, ViewportSize);
		return bCompact ? FMargin(24.0f, 22.0f, 24.0f, 20.0f) : FMargin(42.0f, 38.0f, 42.0f, 34.0f);
	}

	UWidget* FindInventoryWidgetByNames(UWidgetTree* WidgetTree, const TArray<FName>& Names)
	{
		if (!WidgetTree)
		{
			return nullptr;
		}

		for (const FName& Name : Names)
		{
			if (UWidget* Widget = WidgetTree->FindWidget(Name))
			{
				return Widget;
			}
		}
		return nullptr;
	}

	UTextBlock* MakeInventoryText(UWidgetTree* WidgetTree, const FText& Text)
	{
		UTextBlock* TextBlock = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()) : nullptr;
		if (TextBlock)
		{
			TextBlock->SetText(Text);
			TextBlock->SetAutoWrapText(true);
		}
		return TextBlock;
	}

	void AddInventoryButtonText(UWidgetTree* WidgetTree, UButton* Button, const FText& Text)
	{
		if (WidgetTree && Button)
		{
			Button->AddChild(MakeInventoryText(WidgetTree, Text));
		}
	}

	bool TryParseJsonObject(const FString& Json, TSharedPtr<FJsonObject>& OutRoot)
	{
		OutRoot.Reset();
		if (Json.IsEmpty())
		{
			return false;
		}

		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		return FJsonSerializer::Deserialize(Reader, OutRoot) && OutRoot.IsValid();
	}

	FString ReadStringField(const TSharedPtr<FJsonObject>& Object, const TArray<FString>& Keys)
	{
		if (!Object.IsValid())
		{
			return FString();
		}

		for (const FString& Key : Keys)
		{
			FString Value;
			if (Object->TryGetStringField(Key, Value) && !Value.IsEmpty())
			{
				return Value;
			}

			double Number = 0.0;
			if (Object->TryGetNumberField(Key, Number))
			{
				return FString::Printf(TEXT("%.0f"), Number);
			}
		}
		return FString();
	}

	int32 ReadIntField(const TSharedPtr<FJsonObject>& Object, const TArray<FString>& Keys, int32 DefaultValue)
	{
		if (!Object.IsValid())
		{
			return DefaultValue;
		}

		for (const FString& Key : Keys)
		{
			int32 IntValue = 0;
			if (Object->TryGetNumberField(Key, IntValue))
			{
				return IntValue;
			}

			double NumberValue = 0.0;
			if (Object->TryGetNumberField(Key, NumberValue))
			{
				return FMath::RoundToInt(NumberValue);
			}

			FString StringValue;
			if (Object->TryGetStringField(Key, StringValue) && StringValue.IsNumeric())
			{
				return FCString::Atoi(*StringValue);
			}
		}
		return DefaultValue;
	}

	bool ReadBoolField(const TSharedPtr<FJsonObject>& Object, const TArray<FString>& Keys)
	{
		if (!Object.IsValid())
		{
			return false;
		}

		for (const FString& Key : Keys)
		{
			bool bValue = false;
			if (Object->TryGetBoolField(Key, bValue))
			{
				return bValue;
			}
		}
		return false;
	}

	bool LooksLikeInventoryItem(const TSharedPtr<FJsonObject>& Object)
	{
		return Object.IsValid()
			&& (!ReadStringField(Object, { TEXT("itemId"), TEXT("item_id"), TEXT("id"), TEXT("sku"), TEXT("key") }).IsEmpty()
				|| !ReadStringField(Object, { TEXT("displayName"), TEXT("itemName"), TEXT("name"), TEXT("title") }).IsEmpty());
	}

	void CollectInventoryObjects(const TSharedPtr<FJsonObject>& Object, TArray<TSharedPtr<FJsonObject>>& OutObjects, int32 Depth = 0)
	{
		if (!Object.IsValid() || Depth > 8)
		{
			return;
		}

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Object->Values)
		{
			if (!Pair.Value.IsValid())
			{
				continue;
			}

			if (Pair.Value->Type == EJson::Array)
			{
				for (const TSharedPtr<FJsonValue>& Value : Pair.Value->AsArray())
				{
					const TSharedPtr<FJsonObject> ItemObject = Value.IsValid() && Value->Type == EJson::Object ? Value->AsObject() : nullptr;
					if (!ItemObject.IsValid())
					{
						continue;
					}

					if (LooksLikeInventoryItem(ItemObject))
					{
						OutObjects.Add(ItemObject);
					}
					else
					{
						CollectInventoryObjects(ItemObject, OutObjects, Depth + 1);
					}
				}
			}
			else if (Pair.Value->Type == EJson::Object)
			{
				CollectInventoryObjects(Pair.Value->AsObject(), OutObjects, Depth + 1);
			}
		}
	}

	int32 ReadIntByKeysDeep(const TSharedPtr<FJsonObject>& Object, const TArray<FString>& Keys, int32 Depth = 0)
	{
		if (!Object.IsValid() || Depth > 8)
		{
			return 0;
		}

		const int32 DirectValue = ReadIntField(Object, Keys, 0);
		if (DirectValue != 0)
		{
			return DirectValue;
		}

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Object->Values)
		{
			if (!Pair.Value.IsValid())
			{
				continue;
			}
			if (Pair.Value->Type == EJson::Object)
			{
				const int32 NestedValue = ReadIntByKeysDeep(Pair.Value->AsObject(), Keys, Depth + 1);
				if (NestedValue != 0)
				{
					return NestedValue;
				}
			}
		}
		return 0;
	}
}

UDBAInventoryWidgetBase::UDBAInventoryWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	bAutoInjectBackground = false;
	bAutoBindClickSound = false;
	PanelBackgroundTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginPanel_StoneGold.T_DBA_LoginPanel_StoneGold")));
}

void UDBAInventoryWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureNativeFallbackLayout();
	ResolveBoundWidgetsFromWidgetTree();
	ApplyPanelBackgroundTexture();
	BindButtonClickAudio();
	BindControls();
	EnsureController();
	RefreshItemText();
	RefreshDetailsText();
	RefreshInventory();
	SetKeyboardFocus();
}

void UDBAInventoryWidgetBase::NativeDestruct()
{
	UnbindControls();
	if (InventoryController)
	{
		InventoryController->OnInventoryUpdated.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleInventoryUpdated);
		InventoryController->OnBackendError.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleBackendError);
	}
	Super::NativeDestruct();
}

FReply UDBAInventoryWidgetBase::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::B || InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseInventory();
		return FReply::Handled();
	}
	if (InKeyEvent.GetKey() == EKeys::Down)
	{
		SelectNextItem();
		return FReply::Handled();
	}
	if (InKeyEvent.GetKey() == EKeys::Up)
	{
		SelectPreviousItem();
		return FReply::Handled();
	}
	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		UseSelectedItem();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UDBAInventoryWidgetBase::RefreshInventory()
{
	EnsureController();
	if (InventoryController)
	{
		SetStatus(NSLOCTEXT("DBAInventory", "Refreshing", "正在刷新背包..."));
		InventoryController->RefreshPlayerData();
	}
	else
	{
		SetStatus(NSLOCTEXT("DBAInventory", "NoController", "背包服务暂不可用"));
	}
}

void UDBAInventoryWidgetBase::SetInventoryDataFromJson(const FString& InventoryJson)
{
	Items.Reset();

	TSharedPtr<FJsonObject> Root;
	if (TryParseJsonObject(InventoryJson, Root))
	{
		TArray<TSharedPtr<FJsonObject>> ItemObjects;
		CollectInventoryObjects(Root, ItemObjects);
		for (const TSharedPtr<FJsonObject>& ItemObject : ItemObjects)
		{
			FDBAInventoryItemView Item;
			Item.ItemId = ReadStringField(ItemObject, { TEXT("itemId"), TEXT("item_id"), TEXT("id"), TEXT("sku"), TEXT("key") });
			const FString Name = ReadStringField(ItemObject, { TEXT("displayName"), TEXT("itemName"), TEXT("name"), TEXT("title") });
			Item.DisplayName = FText::FromString(Name.IsEmpty() ? Item.ItemId : Name);
			Item.Category = ReadStringField(ItemObject, { TEXT("category"), TEXT("type"), TEXT("kind") });
			Item.Rarity = ReadStringField(ItemObject, { TEXT("rarity"), TEXT("quality"), TEXT("rank") });
			Item.Count = FMath::Max(1, ReadIntField(ItemObject, { TEXT("count"), TEXT("quantity"), TEXT("amount"), TEXT("num"), TEXT("stack") }, 1));
			Item.bUsable = ReadBoolField(ItemObject, { TEXT("usable"), TEXT("canUse"), TEXT("consumable") })
				|| Item.Category.Equals(TEXT("consumable"), ESearchCase::IgnoreCase);
			Items.Add(Item);
		}

		const int32 Gold = ReadIntByKeysDeep(Root, { TEXT("gold"), TEXT("coins"), TEXT("coin"), TEXT("currencyGold") });
		if (Gold > 0)
		{
			FDBAInventoryItemView GoldItem;
			GoldItem.ItemId = TEXT("currency.gold");
			GoldItem.DisplayName = NSLOCTEXT("DBAInventory", "Gold", "金币");
			GoldItem.Category = TEXT("Currency");
			GoldItem.Count = Gold;
			Items.Add(GoldItem);
		}

		const int32 Tickets = ReadIntByKeysDeep(Root, { TEXT("tickets"), TEXT("ticket"), TEXT("matchTickets"), TEXT("coupon") });
		if (Tickets > 0)
		{
			FDBAInventoryItemView TicketItem;
			TicketItem.ItemId = TEXT("currency.ticket");
			TicketItem.DisplayName = NSLOCTEXT("DBAInventory", "Tickets", "门票");
			TicketItem.Category = TEXT("Currency");
			TicketItem.Count = Tickets;
			Items.Add(TicketItem);
		}
	}

	if (Items.Num() == 0)
	{
		SelectedIndex = INDEX_NONE;
		SetStatus(NSLOCTEXT("DBAInventory", "Empty", "背包为空"));
	}
	else
	{
		SelectedIndex = FMath::Clamp(SelectedIndex == INDEX_NONE ? 0 : SelectedIndex, 0, Items.Num() - 1);
		SetStatus(NSLOCTEXT("DBAInventory", "Loaded", "背包已更新"));
	}

	RefreshItemText();
	RefreshDetailsText();
	BP_OnInventoryUpdated(Items, SelectedIndex);
}

void UDBAInventoryWidgetBase::SortInventory()
{
	Items.Sort(
		[](const FDBAInventoryItemView& A, const FDBAInventoryItemView& B)
		{
			const int32 CategoryCompare = A.Category.Compare(B.Category);
			if (CategoryCompare != 0)
			{
				return CategoryCompare < 0;
			}
			return A.DisplayName.ToString() < B.DisplayName.ToString();
		});
	SelectedIndex = Items.Num() > 0 ? 0 : INDEX_NONE;
	RefreshItemText();
	RefreshDetailsText();
	SetStatus(NSLOCTEXT("DBAInventory", "Sorted", "背包已排序"));
	BP_OnInventoryUpdated(Items, SelectedIndex);
}

void UDBAInventoryWidgetBase::SelectItemByIndex(int32 ItemIndex)
{
	if (!Items.IsValidIndex(ItemIndex))
	{
		return;
	}
	SelectedIndex = ItemIndex;
	RefreshItemText();
	RefreshDetailsText();
	BP_OnInventoryUpdated(Items, SelectedIndex);
}

void UDBAInventoryWidgetBase::SelectNextItem()
{
	if (Items.Num() == 0)
	{
		return;
	}
	SelectItemByIndex(SelectedIndex == INDEX_NONE ? 0 : (SelectedIndex + 1) % Items.Num());
}

void UDBAInventoryWidgetBase::SelectPreviousItem()
{
	if (Items.Num() == 0)
	{
		return;
	}
	SelectItemByIndex(SelectedIndex == INDEX_NONE ? 0 : (SelectedIndex - 1 + Items.Num()) % Items.Num());
}

void UDBAInventoryWidgetBase::UseSelectedItem()
{
	if (!Items.IsValidIndex(SelectedIndex))
	{
		SetStatus(NSLOCTEXT("DBAInventory", "NoSelection", "请选择物品"));
		return;
	}

	const FDBAInventoryItemView& Item = Items[SelectedIndex];
	if (!Item.bUsable)
	{
		SetStatus(NSLOCTEXT("DBAInventory", "NotUsable", "该物品不能直接使用"));
		return;
	}

	SetStatus(FText::Format(NSLOCTEXT("DBAInventory", "UsePending", "已请求使用：{0}"), Item.DisplayName));
}

void UDBAInventoryWidgetBase::CloseInventory()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->HideInventory();
			return;
		}
	}
	RemoveFromParent();
}

void UDBAInventoryWidgetBase::EnsureNativeFallbackLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("NativeInventoryCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	PanelBackgroundBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PanelBackgroundBorder"));
	PanelBackgroundBorder->SetPadding(ResolveResponsiveInventoryPanelPadding(this));
	PanelBackgroundBorder->SetBrushColor(FLinearColor(0.08f, 0.065f, 0.045f, 0.96f));

	if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelBackgroundBorder))
	{
		const FVector2D PanelSize = ResolveResponsiveInventoryPanelSize(this, NativePanelSize);
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetOffsets(FMargin(0.0f, 0.0f, PanelSize.X, PanelSize.Y));
		PanelSlot->SetZOrder(10);
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NativeInventoryRoot"));
	PanelBackgroundBorder->SetContent(RootBox);
	RootBox->AddChildToVerticalBox(MakeInventoryText(WidgetTree, NSLOCTEXT("DBAInventory", "Title", "背包")));

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	RootBox->AddChildToVerticalBox(StatusText);

	ItemsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ItemsText"));
	ItemsText->SetAutoWrapText(true);
	RootBox->AddChildToVerticalBox(ItemsText);

	DetailsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailsText"));
	DetailsText->SetAutoWrapText(true);
	RootBox->AddChildToVerticalBox(DetailsText);

	UHorizontalBox* ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	RootBox->AddChildToVerticalBox(ButtonRow);

	PreviousButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PreviousButton"));
	AddInventoryButtonText(WidgetTree, PreviousButton, NSLOCTEXT("DBAInventory", "Previous", "上一个"));
	ButtonRow->AddChildToHorizontalBox(PreviousButton);

	NextButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("NextButton"));
	AddInventoryButtonText(WidgetTree, NextButton, NSLOCTEXT("DBAInventory", "Next", "下一个"));
	ButtonRow->AddChildToHorizontalBox(NextButton);

	UseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("UseButton"));
	AddInventoryButtonText(WidgetTree, UseButton, NSLOCTEXT("DBAInventory", "Use", "使用"));
	ButtonRow->AddChildToHorizontalBox(UseButton);

	SortButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SortButton"));
	AddInventoryButtonText(WidgetTree, SortButton, NSLOCTEXT("DBAInventory", "Sort", "排序"));
	ButtonRow->AddChildToHorizontalBox(SortButton);

	RefreshButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RefreshButton"));
	AddInventoryButtonText(WidgetTree, RefreshButton, NSLOCTEXT("DBAInventory", "Refresh", "刷新"));
	ButtonRow->AddChildToHorizontalBox(RefreshButton);

	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
	AddInventoryButtonText(WidgetTree, CloseButton, NSLOCTEXT("DBAInventory", "Close", "关闭"));
	ButtonRow->AddChildToHorizontalBox(CloseButton);
}

void UDBAInventoryWidgetBase::ResolveBoundWidgetsFromWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!CloseButton) { CloseButton = Cast<UButton>(FindInventoryWidgetByNames(WidgetTree, { TEXT("CloseButton"), TEXT("BackButton") })); }
	if (!RefreshButton) { RefreshButton = Cast<UButton>(FindInventoryWidgetByNames(WidgetTree, { TEXT("RefreshButton"), TEXT("ReloadButton") })); }
	if (!SortButton) { SortButton = Cast<UButton>(FindInventoryWidgetByNames(WidgetTree, { TEXT("SortButton"), TEXT("ArrangeButton") })); }
	if (!UseButton) { UseButton = Cast<UButton>(FindInventoryWidgetByNames(WidgetTree, { TEXT("UseButton"), TEXT("EquipButton") })); }
	if (!NextButton) { NextButton = Cast<UButton>(FindInventoryWidgetByNames(WidgetTree, { TEXT("NextButton") })); }
	if (!PreviousButton) { PreviousButton = Cast<UButton>(FindInventoryWidgetByNames(WidgetTree, { TEXT("PreviousButton"), TEXT("PrevButton") })); }
	if (!ItemsText) { ItemsText = Cast<UTextBlock>(FindInventoryWidgetByNames(WidgetTree, { TEXT("ItemsText"), TEXT("ItemListText"), TEXT("InventoryItemsText") })); }
	if (!DetailsText) { DetailsText = Cast<UTextBlock>(FindInventoryWidgetByNames(WidgetTree, { TEXT("DetailsText"), TEXT("ItemDetailsText") })); }
	if (!StatusText) { StatusText = Cast<UTextBlock>(FindInventoryWidgetByNames(WidgetTree, { TEXT("StatusText"), TEXT("InventoryStatusText") })); }
	if (!PanelBackgroundBorder)
	{
		PanelBackgroundBorder = Cast<UBorder>(FindInventoryWidgetByNames(WidgetTree, {
			TEXT("PanelBackgroundBorder"),
			TEXT("InventoryPanelBackground"),
			TEXT("InventoryPanel"),
			TEXT("RootPanel")
		}));
	}
	if (!PanelBackgroundImage)
	{
		PanelBackgroundImage = Cast<UImage>(FindInventoryWidgetByNames(WidgetTree, {
			TEXT("PanelBackgroundImage"),
			TEXT("InventoryPanelBackgroundImage"),
			TEXT("InventoryBackgroundImage")
		}));
	}
}

void UDBAInventoryWidgetBase::ApplyPanelBackgroundTexture()
{
	UTexture2D* Texture = PanelBackgroundTexture.LoadSynchronous();
	if (PanelBackgroundBorder)
	{
		if (Texture)
		{
			PanelBackgroundBorder->SetBrushFromTexture(Texture);
			PanelBackgroundBorder->SetBrushColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.96f));
		}
		else
		{
			PanelBackgroundBorder->SetBrushColor(FLinearColor(0.08f, 0.065f, 0.045f, 0.96f));
		}
	}
	if (PanelBackgroundImage)
	{
		if (Texture)
		{
			PanelBackgroundImage->SetBrushFromTexture(Texture, true);
			PanelBackgroundImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.96f));
		}
		else
		{
			PanelBackgroundImage->SetColorAndOpacity(FLinearColor(0.08f, 0.065f, 0.045f, 0.96f));
		}
	}
}

void UDBAInventoryWidgetBase::BindControls()
{
	if (CloseButton) { CloseButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleCloseClicked); CloseButton->OnClicked.AddDynamic(this, &UDBAInventoryWidgetBase::HandleCloseClicked); }
	if (RefreshButton) { RefreshButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleRefreshClicked); RefreshButton->OnClicked.AddDynamic(this, &UDBAInventoryWidgetBase::HandleRefreshClicked); }
	if (SortButton) { SortButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleSortClicked); SortButton->OnClicked.AddDynamic(this, &UDBAInventoryWidgetBase::HandleSortClicked); }
	if (UseButton) { UseButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleUseClicked); UseButton->OnClicked.AddDynamic(this, &UDBAInventoryWidgetBase::HandleUseClicked); }
	if (NextButton) { NextButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleNextClicked); NextButton->OnClicked.AddDynamic(this, &UDBAInventoryWidgetBase::HandleNextClicked); }
	if (PreviousButton) { PreviousButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandlePreviousClicked); PreviousButton->OnClicked.AddDynamic(this, &UDBAInventoryWidgetBase::HandlePreviousClicked); }
}

void UDBAInventoryWidgetBase::UnbindControls()
{
	if (CloseButton) { CloseButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleCloseClicked); }
	if (RefreshButton) { RefreshButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleRefreshClicked); }
	if (SortButton) { SortButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleSortClicked); }
	if (UseButton) { UseButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleUseClicked); }
	if (NextButton) { NextButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleNextClicked); }
	if (PreviousButton) { PreviousButton->OnClicked.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandlePreviousClicked); }
}

void UDBAInventoryWidgetBase::EnsureController()
{
	if (!InventoryController)
	{
		InventoryController = NewObject<UDBAMainLobbyWidgetController>(this, UDBAMainLobbyWidgetController::StaticClass());
		if (InventoryController)
		{
			InventoryController->InitializeController();
		}
	}

	if (InventoryController)
	{
		InventoryController->OnInventoryUpdated.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleInventoryUpdated);
		InventoryController->OnInventoryUpdated.AddDynamic(this, &UDBAInventoryWidgetBase::HandleInventoryUpdated);
		InventoryController->OnBackendError.RemoveDynamic(this, &UDBAInventoryWidgetBase::HandleBackendError);
		InventoryController->OnBackendError.AddDynamic(this, &UDBAInventoryWidgetBase::HandleBackendError);
	}
}

void UDBAInventoryWidgetBase::RefreshItemText()
{
	if (!ItemsText)
	{
		return;
	}

	if (Items.Num() == 0)
	{
		ItemsText->SetText(NSLOCTEXT("DBAInventory", "NoItems", "暂无物品"));
		return;
	}

	TArray<FString> Lines;
	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		const FDBAInventoryItemView& Item = Items[Index];
		const TCHAR* Prefix = Index == SelectedIndex ? TEXT(">") : TEXT(" ");
		Lines.Add(FString::Printf(TEXT("%s %s x%d  %s"),
			Prefix,
			*Item.DisplayName.ToString(),
			Item.Count,
			Item.Rarity.IsEmpty() ? TEXT("") : *Item.Rarity));
	}
	ItemsText->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
}

void UDBAInventoryWidgetBase::RefreshDetailsText()
{
	if (!DetailsText)
	{
		return;
	}

	if (!Items.IsValidIndex(SelectedIndex))
	{
		DetailsText->SetText(NSLOCTEXT("DBAInventory", "NoDetails", "未选择物品"));
		return;
	}

	const FDBAInventoryItemView& Item = Items[SelectedIndex];
	DetailsText->SetText(FText::FromString(FString::Printf(
		TEXT("名称：%s\n数量：%d\n类型：%s\n品质：%s\nID：%s"),
		*Item.DisplayName.ToString(),
		Item.Count,
		Item.Category.IsEmpty() ? TEXT("未分类") : *Item.Category,
		Item.Rarity.IsEmpty() ? TEXT("普通") : *Item.Rarity,
		Item.ItemId.IsEmpty() ? TEXT("-") : *Item.ItemId)));
}

void UDBAInventoryWidgetBase::SetStatus(const FText& Text)
{
	if (StatusText)
	{
		StatusText->SetText(Text);
	}
}

void UDBAInventoryWidgetBase::HandleInventoryUpdated(const FString& DataJson)
{
	SetInventoryDataFromJson(DataJson);
}

void UDBAInventoryWidgetBase::HandleBackendError(const FString& ErrorMessage)
{
	SetStatus(FText::FromString(ErrorMessage.IsEmpty() ? TEXT("背包刷新失败") : ErrorMessage));
}

void UDBAInventoryWidgetBase::HandleCloseClicked()
{
	CloseInventory();
}

void UDBAInventoryWidgetBase::HandleRefreshClicked()
{
	RefreshInventory();
}

void UDBAInventoryWidgetBase::HandleSortClicked()
{
	SortInventory();
}

void UDBAInventoryWidgetBase::HandleUseClicked()
{
	UseSelectedItem();
}

void UDBAInventoryWidgetBase::HandleNextClicked()
{
	SelectNextItem();
}

void UDBAInventoryWidgetBase::HandlePreviousClicked()
{
	SelectPreviousItem();
}
