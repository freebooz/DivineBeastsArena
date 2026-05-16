// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/UDBALobbyPlayerHUDWidgetBase.h"

#include "EngineUtils.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SafeZone.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/DataTable.h"
#include "GameCore/Account/DBAOnlineAccountService.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Data/DBASkillDataRow.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Styling/CoreStyle.h"

namespace
{
	bool IsMobilePlatform()
	{
		const FString Platform = UGameplayStatics::GetPlatformName();
		return Platform.Contains(TEXT("Android"), ESearchCase::IgnoreCase)
			|| Platform.Contains(TEXT("IOS"), ESearchCase::IgnoreCase);
	}

	EDBAElement ResolveSummaryElement(const FDBACharacterSummary& Summary)
	{
		return Summary.PrimaryElement == EDBAElement::None
			? Summary.DefaultElement
			: Summary.PrimaryElement;
	}

	FString ElementSkillPrefix(EDBAElement Element)
	{
		switch (Element)
		{
		case EDBAElement::Fire: return TEXT("Fire");
		case EDBAElement::Water: return TEXT("Water");
		case EDBAElement::Wood: return TEXT("Wood");
		case EDBAElement::Gold: return TEXT("Gold");
		case EDBAElement::Earth: return TEXT("Earth");
		default: return TEXT("Unknown");
		}
	}
}

UDBALobbyPlayerHUDWidgetBase::UDBALobbyPlayerHUDWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoInjectBackground = false;
	bAutoBindClickSound = false;
}

void UDBALobbyPlayerHUDWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildDefaultLayoutIfNeeded();
}

void UDBALobbyPlayerHUDWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshFromCurrentCharacterData();
}

void UDBALobbyPlayerHUDWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
	if (!ViewportSize.Equals(LastResponsiveViewport, 0.5f))
	{
		ApplyResponsiveLayout(ViewportSize);
		LastResponsiveViewport = ViewportSize;
	}
	UpdateMinimap();
}

void UDBALobbyPlayerHUDWidgetBase::RefreshFromCurrentCharacterData()
{
	FDBACharacterSummary Summary;
	if (!ResolveCurrentCharacterSummary(Summary))
	{
		if (AvatarNameText)
		{
			AvatarNameText->SetText(FText::FromString(TEXT("Player")));
		}
		if (AvatarMetaText)
		{
			AvatarMetaText->SetText(FText::FromString(TEXT("No Character Data")));
		}
		ApplySkillLabels({
			FText::FromString(TEXT("Skill 1")),
			FText::FromString(TEXT("Skill 2")),
			FText::FromString(TEXT("Skill 3")),
			FText::FromString(TEXT("Skill 4"))});
		return;
	}

	const EDBAZodiac Zodiac = Summary.Zodiac == EDBAZodiac::None ? Summary.DefaultZodiac : Summary.Zodiac;
	const EDBAElement Element = ResolveSummaryElement(Summary);

	if (AvatarNameText)
	{
		AvatarNameText->SetText(FText::FromString(Summary.CharacterName.IsEmpty() ? TEXT("Player") : Summary.CharacterName));
	}
	if (AvatarMetaText)
	{
		const FString MetaString = FString::Printf(TEXT("%s | %s | Lv.%d"),
			*ZodiacToShortText(Zodiac).ToString(),
			*ElementToShortText(Element).ToString(),
			FMath::Max(1, Summary.Level));
		AvatarMetaText->SetText(FText::FromString(MetaString));
	}
	if (AvatarImage)
	{
		AvatarImage->SetColorAndOpacity(ZodiacToColor(Zodiac));
	}

	TArray<FText> SkillLabels;
	ResolveSkillLabelsForSummary(Summary, SkillLabels);
	ApplySkillLabels(SkillLabels);
}

void UDBALobbyPlayerHUDWidgetBase::BuildDefaultLayoutIfNeeded()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!RootCanvasPanel)
	{
		if (UCanvasPanel* ExistingCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget))
		{
			RootCanvasPanel = ExistingCanvas;
		}
		else if (USafeZone* ExistingSafeZone = Cast<USafeZone>(WidgetTree->RootWidget))
		{
			RootCanvasPanel = Cast<UCanvasPanel>(ExistingSafeZone->GetChildAt(0));
		}
	}

	if (!RootCanvasPanel)
	{
		USafeZone* SafeZone = WidgetTree->ConstructWidget<USafeZone>(USafeZone::StaticClass(), TEXT("LobbyHUDSafeZone"));
		RootCanvasPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("LobbyHUDCanvas"));
		SafeZone->AddChild(RootCanvasPanel);
		WidgetTree->RootWidget = SafeZone;
	}

	if (!AvatarRootBorder)
	{
		BuildTopLeftAvatarPanel(RootCanvasPanel);
	}
	if (SkillSlotBorders.Num() == 0)
	{
		BuildBottomSkillBar(RootCanvasPanel);
	}
	if (!MinimapRootBorder)
	{
		BuildTopRightMinimap(RootCanvasPanel);
	}
}

void UDBALobbyPlayerHUDWidgetBase::BuildTopLeftAvatarPanel(UCanvasPanel* RootCanvas)
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	const bool bMobile = IsMobilePlatform();
	const FVector2D LocalPanelSize = bMobile ? AvatarPanelSize * 0.85f : AvatarPanelSize;

	AvatarRootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyHUD_AvatarRoot"));
	AvatarRootBorder->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.76f));
	AvatarRootBorder->SetPadding(FMargin(10.0f));

	UCanvasPanelSlot* RootSlot = RootCanvas->AddChildToCanvas(AvatarRootBorder);
	AvatarRootSlot = RootSlot;
	RootSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	RootSlot->SetAlignment(FVector2D::ZeroVector);
	RootSlot->SetPosition(FVector2D(18.0f, 18.0f));
	RootSlot->SetSize(LocalPanelSize);
	RootSlot->SetZOrder(40);

	UHorizontalBox* RootHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("LobbyHUD_AvatarHBox"));
	AvatarRootBorder->SetContent(RootHBox);

	AvatarImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("LobbyHUD_AvatarImage"));
	AvatarImage->SetColorAndOpacity(FLinearColor(0.18f, 0.48f, 1.0f, 1.0f));
	if (UHorizontalBoxSlot* ImageSlot = RootHBox->AddChildToHorizontalBox(AvatarImage))
	{
		const float SquareSize = LocalPanelSize.Y - 22.0f;
		ImageSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		ImageSlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));
		AvatarImage->SetDesiredSizeOverride(FVector2D(SquareSize, SquareSize));
	}

	UVerticalBox* MetaVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyHUD_AvatarMetaVBox"));
	if (UHorizontalBoxSlot* VBoxSlot = RootHBox->AddChildToHorizontalBox(MetaVBox))
	{
		VBoxSlot->SetHorizontalAlignment(HAlign_Left);
		VBoxSlot->SetVerticalAlignment(VAlign_Center);
	}

	AvatarNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyHUD_AvatarNameText"));
	AvatarNameText->SetText(FText::FromString(TEXT("Player")));
	AvatarNameText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), bMobile ? 18 : 20));
	AvatarNameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	MetaVBox->AddChildToVerticalBox(AvatarNameText);

	AvatarMetaText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyHUD_AvatarMetaText"));
	AvatarMetaText->SetText(FText::FromString(TEXT("Zodiac | Element | Lv.1")));
	AvatarMetaText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), bMobile ? 13 : 14));
	AvatarMetaText->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 0.84f, 0.9f, 1.0f)));
	MetaVBox->AddChildToVerticalBox(AvatarMetaText);
}

void UDBALobbyPlayerHUDWidgetBase::BuildBottomSkillBar(UCanvasPanel* RootCanvas)
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	const bool bMobile = IsMobilePlatform();
	const FVector2D LocalSlotSize = bMobile ? SkillSlotSize * FVector2D(0.9f, 0.95f) : SkillSlotSize;

	UHorizontalBox* SkillBarHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("LobbyHUD_SkillBarHBox"));
	UCanvasPanelSlot* SkillBarSlot = RootCanvas->AddChildToCanvas(SkillBarHBox);
	SkillBarRootSlot = SkillBarSlot;
	SkillBarSlot->SetAnchors(FAnchors(0.5f, 1.0f));
	SkillBarSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	SkillBarSlot->SetPosition(FVector2D(0.0f, bMobile ? -10.0f : -18.0f));
	SkillBarSlot->SetAutoSize(true);
	SkillBarSlot->SetZOrder(45);

	static const TCHAR* DesktopHotkeys[4] = { TEXT("Q"), TEXT("W"), TEXT("E"), TEXT("R") };
	static const TCHAR* MobileHotkeys[4] = { TEXT("S1"), TEXT("S2"), TEXT("S3"), TEXT("S4") };
	const TCHAR* const* Hotkeys = bMobile ? MobileHotkeys : DesktopHotkeys;
	SkillSlotBorders.Reset();
	SkillNameTexts.Reset();
	SkillHotkeyTexts.Reset();
	SkillCooldownTexts.Reset();

	for (int32 Index = 0; Index < 4; ++Index)
	{
		UBorder* SlotBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillSlot_%d"), Index));
		SlotBorder->SetBrushColor(FLinearColor(0.04f, 0.04f, 0.05f, 0.82f));
		SlotBorder->SetPadding(FMargin(8.0f));

		if (UHorizontalBoxSlot* SkillHBoxSlot = SkillBarHBox->AddChildToHorizontalBox(SlotBorder))
		{
			SkillHBoxSlot->SetPadding(FMargin(6.0f, 0.0f));
			SkillHBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		}

		UOverlay* SlotOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillOverlay_%d"), Index));
		SlotBorder->SetContent(SlotOverlay);

		UVerticalBox* ContentVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillVBox_%d"), Index));
		if (UOverlaySlot* OverlaySlot = SlotOverlay->AddChildToOverlay(ContentVBox))
		{
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}

		UTextBlock* NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillName_%d"), Index));
		NameText->SetText(FText::FromString(FString::Printf(TEXT("Skill %d"), Index + 1)));
		NameText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), bMobile ? 12 : 13));
		NameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.95f, 0.98f, 1.0f)));
		if (UVerticalBoxSlot* NameSlot = ContentVBox->AddChildToVerticalBox(NameText))
		{
			NameSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
		}

		UTextBlock* CooldownText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillCD_%d"), Index));
		CooldownText->SetText(FText::FromString(TEXT("Ready")));
		CooldownText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), bMobile ? 11 : 12));
		CooldownText->SetColorAndOpacity(FSlateColor(FLinearColor(0.52f, 0.95f, 0.72f, 1.0f)));
		ContentVBox->AddChildToVerticalBox(CooldownText);

		UTextBlock* HotkeyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillHotkey_%d"), Index));
		HotkeyText->SetText(FText::FromString(Hotkeys[Index]));
		HotkeyText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), bMobile ? 13 : 14));
		HotkeyText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.84f, 0.18f, 1.0f)));
		if (UOverlaySlot* KeyOverlaySlot = SlotOverlay->AddChildToOverlay(HotkeyText))
		{
			KeyOverlaySlot->SetHorizontalAlignment(HAlign_Right);
			KeyOverlaySlot->SetVerticalAlignment(VAlign_Bottom);
			KeyOverlaySlot->SetPadding(FMargin(0.0f, 0.0f, 4.0f, 2.0f));
		}

		SlotBorder->SetDesiredSizeScale(FVector2D(LocalSlotSize.X / SkillSlotSize.X, LocalSlotSize.Y / SkillSlotSize.Y));

		SkillSlotBorders.Add(SlotBorder);
		SkillNameTexts.Add(NameText);
		SkillCooldownTexts.Add(CooldownText);
		SkillHotkeyTexts.Add(HotkeyText);
	}
}

void UDBALobbyPlayerHUDWidgetBase::BuildTopRightMinimap(UCanvasPanel* RootCanvas)
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	const bool bMobile = IsMobilePlatform();
	const FVector2D LocalMinimapSize = bMobile ? MinimapSize * 0.85f : MinimapSize;

	MinimapRootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyHUD_MinimapRoot"));
	MinimapRootBorder->SetBrushColor(FLinearColor(0.03f, 0.03f, 0.04f, 0.78f));
	MinimapRootBorder->SetPadding(FMargin(8.0f));

	UCanvasPanelSlot* MinimapSlot = RootCanvas->AddChildToCanvas(MinimapRootBorder);
	MinimapRootSlot = MinimapSlot;
	MinimapSlot->SetAnchors(FAnchors(1.0f, 0.0f));
	MinimapSlot->SetAlignment(FVector2D(1.0f, 0.0f));
	MinimapSlot->SetPosition(FVector2D(-18.0f, 18.0f));
	MinimapSlot->SetSize(LocalMinimapSize);
	MinimapSlot->SetZOrder(42);

	UOverlay* MinimapOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("LobbyHUD_MinimapOverlay"));
	MinimapRootBorder->SetContent(MinimapOverlay);

	UImage* MapBackImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("LobbyHUD_MinimapBackImage"));
	MapBackImage->SetColorAndOpacity(FLinearColor(0.08f, 0.14f, 0.1f, 0.9f));
	if (UOverlaySlot* BackSlot = MinimapOverlay->AddChildToOverlay(MapBackImage))
	{
		BackSlot->SetHorizontalAlignment(HAlign_Fill);
		BackSlot->SetVerticalAlignment(VAlign_Fill);
	}

	MinimapDotCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("LobbyHUD_MinimapDotCanvas"));
	if (UOverlaySlot* DotSlot = MinimapOverlay->AddChildToOverlay(MinimapDotCanvas))
	{
		DotSlot->SetHorizontalAlignment(HAlign_Fill);
		DotSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UDBALobbyPlayerHUDWidgetBase::UpdateMinimap()
{
	if (!MinimapDotCanvas || !WidgetTree)
	{
		return;
	}

	UWorld* World = GetWorld();
	APlayerController* PC = GetOwningPlayer();
	APawn* LocalPawn = PC ? PC->GetPawn() : nullptr;
	if (!World || !LocalPawn)
	{
		return;
	}

	if (!bMinimapOriginInitialized)
	{
		MinimapOriginWorld = LocalPawn->GetActorLocation();
		bMinimapOriginInitialized = true;
	}

	TArray<ADBAZodiacCharacterBase*> TrackedPlayers;
	for (TActorIterator<ADBAZodiacCharacterBase> It(World); It; ++It)
	{
		if (ADBAZodiacCharacterBase* Character = *It)
		{
			TrackedPlayers.Add(Character);
		}
	}

	const int32 TrackCount = FMath::Min(MaxMinimapTrackedPlayers, TrackedPlayers.Num());
	while (MinimapDots.Num() < TrackCount)
	{
		UImage* Dot = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *FString::Printf(TEXT("LobbyHUD_MinimapDot_%d"), MinimapDots.Num()));
		Dot->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.75f, 0.95f));
		if (UCanvasPanelSlot* DotSlot = MinimapDotCanvas->AddChildToCanvas(Dot))
		{
			DotSlot->SetSize(FVector2D(9.0f, 9.0f));
			DotSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		}
		MinimapDots.Add(Dot);
	}

	const FVector2D MapPixelSize = MinimapRootBorder ? MinimapRootBorder->GetCachedGeometry().GetLocalSize() : MinimapSize;
	for (int32 Index = 0; Index < MinimapDots.Num(); ++Index)
	{
		if (UImage* Dot = MinimapDots[Index])
		{
			Dot->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	for (int32 Index = 0; Index < TrackCount; ++Index)
	{
		ADBAZodiacCharacterBase* Character = TrackedPlayers[Index];
		UImage* Dot = MinimapDots[Index];
		if (!Character || !Dot)
		{
			continue;
		}

		const FVector Relative = Character->GetActorLocation() - MinimapOriginWorld;
		const float HalfRange = FMath::Max(1.0f, MinimapWorldRange * 0.5f);
		const float NormalizedX = FMath::Clamp(Relative.Y / HalfRange, -1.0f, 1.0f);
		const float NormalizedY = FMath::Clamp(Relative.X / HalfRange, -1.0f, 1.0f);

		const FVector2D PixelPos(
			(0.5f + 0.46f * NormalizedX) * MapPixelSize.X,
			(0.5f - 0.46f * NormalizedY) * MapPixelSize.Y);

		if (UCanvasPanelSlot* DotSlot = Cast<UCanvasPanelSlot>(Dot->Slot))
		{
			DotSlot->SetPosition(PixelPos);
		}

		const bool bIsSelf = Character == LocalPawn;
		Dot->SetColorAndOpacity(bIsSelf
			? FLinearColor(0.05f, 1.0f, 0.42f, 1.0f)
			: FLinearColor(1.0f, 0.72f, 0.16f, 0.95f));
		Dot->SetVisibility(ESlateVisibility::Visible);
	}
}

void UDBALobbyPlayerHUDWidgetBase::ApplyResponsiveLayout(const FVector2D& ViewportSize)
{
	if (ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f)
	{
		return;
	}

	const bool bMobilePlatform = IsMobilePlatform();
	const float MinDimension = FMath::Min(ViewportSize.X, ViewportSize.Y);
	const bool bCompactViewport = MinDimension < 900.0f || ViewportSize.X < 1400.0f;
	const bool bMobileLike = bMobilePlatform || bCompactViewport;

	const float BaseScale = bMobileLike
		? FMath::Clamp(MinDimension / 900.0f, 0.72f, 0.95f)
		: 1.0f;

	if (AvatarRootSlot)
	{
		AvatarRootSlot->SetPosition(FVector2D(18.0f, 18.0f));
		AvatarRootSlot->SetSize(AvatarPanelSize * BaseScale);
	}

	if (SkillBarRootSlot)
	{
		SkillBarRootSlot->SetPosition(FVector2D(0.0f, bMobileLike ? -8.0f : -18.0f));
	}

	if (MinimapRootSlot)
	{
		MinimapRootSlot->SetPosition(FVector2D(-18.0f, 18.0f));
		MinimapRootSlot->SetSize(MinimapSize * (bMobileLike ? BaseScale : 1.0f));
	}

	const float SkillScale = bMobileLike ? FMath::Clamp(BaseScale + 0.06f, 0.80f, 1.0f) : 1.0f;
	for (int32 Index = 0; Index < SkillSlotBorders.Num(); ++Index)
	{
		if (UBorder* Border = SkillSlotBorders[Index])
		{
			Border->SetDesiredSizeScale(FVector2D(SkillScale, SkillScale));
		}

		if (UTextBlock* HotkeyText = SkillHotkeyTexts.IsValidIndex(Index) ? SkillHotkeyTexts[Index] : nullptr)
		{
			HotkeyText->SetText(FText::FromString(bMobileLike
				? FString::Printf(TEXT("S%d"), Index + 1)
				: (Index == 0 ? TEXT("Q") : Index == 1 ? TEXT("W") : Index == 2 ? TEXT("E") : TEXT("R"))));
		}
	}
}

void UDBALobbyPlayerHUDWidgetBase::ApplySkillLabels(const TArray<FText>& SkillLabels)
{
	for (int32 Index = 0; Index < SkillNameTexts.Num(); ++Index)
	{
		if (UTextBlock* NameText = SkillNameTexts[Index])
		{
			const FText Label = SkillLabels.IsValidIndex(Index) ? SkillLabels[Index] : FText::FromString(FString::Printf(TEXT("Skill %d"), Index + 1));
			NameText->SetText(Label);
		}
		if (UTextBlock* CooldownText = SkillCooldownTexts.IsValidIndex(Index) ? SkillCooldownTexts[Index] : nullptr)
		{
			CooldownText->SetText(FText::FromString(TEXT("Ready")));
		}
	}
}

void UDBALobbyPlayerHUDWidgetBase::ResolveSkillLabelsForSummary(const FDBACharacterSummary& Summary, TArray<FText>& OutSkillLabels) const
{
	OutSkillLabels = {
		FText::FromString(TEXT("Skill 1")),
		FText::FromString(TEXT("Skill 2")),
		FText::FromString(TEXT("Skill 3")),
		FText::FromString(TEXT("Skill 4"))
	};

	const EDBAElement Element = ResolveSummaryElement(Summary);
	if (Element == EDBAElement::None)
	{
		return;
	}

	UDataTable* SkillDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/Skills/SkillDataTable.SkillDataTable"));
	if (!SkillDataTable)
	{
		return;
	}

	const FString ElementToken = ElementSkillPrefix(Element);
	for (int32 SkillIndex = 0; SkillIndex < 4; ++SkillIndex)
	{
		const FName RowName(*FString::Printf(TEXT("Element_%s_Skill_%d"), *ElementToken, SkillIndex + 1));
		if (const FDBASkillDataRow* Row = SkillDataTable->FindRow<FDBASkillDataRow>(RowName, TEXT("LobbyHUD")))
		{
			if (!Row->DisplayName.IsEmpty())
			{
				OutSkillLabels[SkillIndex] = Row->DisplayName;
			}
			else
			{
				OutSkillLabels[SkillIndex] = FText::FromName(RowName);
			}
		}
	}
}

bool UDBALobbyPlayerHUDWidgetBase::ResolveCurrentCharacterSummary(FDBACharacterSummary& OutSummary) const
{
	OutSummary = FDBACharacterSummary();

	const UWorld* World = GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	if (!GameInstance)
	{
		return false;
	}

	const UDBALoginFlowSubsystem* LoginFlow = GameInstance->GetSubsystem<UDBALoginFlowSubsystem>();
	if (!LoginFlow)
	{
		return false;
	}

	const TArray<FDBACharacterSummary>& Characters = LoginFlow->GetCachedCharacters();
	if (Characters.Num() <= 0)
	{
		return false;
	}

	const UDBAOnlineAccountService* AccountService = GameInstance->GetSubsystem<UDBAOnlineAccountService>();
	const FDBACharacterId CurrentCharacterId = AccountService ? AccountService->GetCurrentCharacterId() : FDBACharacterId();

	const FDBACharacterSummary* SelectedCharacter = Characters.FindByPredicate(
		[&CurrentCharacterId](const FDBACharacterSummary& Candidate)
		{
			return CurrentCharacterId.IsValid() && Candidate.CharacterId == CurrentCharacterId;
		});

	if (!SelectedCharacter)
	{
		SelectedCharacter = &Characters[0];
	}

	OutSummary = *SelectedCharacter;
	return true;
}

FText UDBALobbyPlayerHUDWidgetBase::ZodiacToShortText(EDBAZodiac Zodiac)
{
	switch (Zodiac)
	{
	case EDBAZodiac::Rat: return FText::FromString(TEXT("Rat"));
	case EDBAZodiac::Ox: return FText::FromString(TEXT("Ox"));
	case EDBAZodiac::Tiger: return FText::FromString(TEXT("Tiger"));
	case EDBAZodiac::Rabbit: return FText::FromString(TEXT("Rabbit"));
	case EDBAZodiac::Dragon: return FText::FromString(TEXT("Dragon"));
	case EDBAZodiac::Snake: return FText::FromString(TEXT("Snake"));
	case EDBAZodiac::Horse: return FText::FromString(TEXT("Horse"));
	case EDBAZodiac::Goat: return FText::FromString(TEXT("Goat"));
	case EDBAZodiac::Monkey: return FText::FromString(TEXT("Monkey"));
	case EDBAZodiac::Rooster: return FText::FromString(TEXT("Rooster"));
	case EDBAZodiac::Dog: return FText::FromString(TEXT("Dog"));
	case EDBAZodiac::Pig: return FText::FromString(TEXT("Pig"));
	default: return FText::FromString(TEXT("Unknown"));
	}
}

FText UDBALobbyPlayerHUDWidgetBase::ElementToShortText(EDBAElement Element)
{
	switch (Element)
	{
	case EDBAElement::Fire: return FText::FromString(TEXT("Fire"));
	case EDBAElement::Water: return FText::FromString(TEXT("Water"));
	case EDBAElement::Wood: return FText::FromString(TEXT("Wood"));
	case EDBAElement::Gold: return FText::FromString(TEXT("Gold"));
	case EDBAElement::Earth: return FText::FromString(TEXT("Earth"));
	default: return FText::FromString(TEXT("None"));
	}
}

FLinearColor UDBALobbyPlayerHUDWidgetBase::ZodiacToColor(EDBAZodiac Zodiac)
{
	switch (Zodiac)
	{
	case EDBAZodiac::Rat: return FLinearColor(0.18f, 0.48f, 1.00f, 1.0f);
	case EDBAZodiac::Ox: return FLinearColor(0.96f, 0.52f, 0.12f, 1.0f);
	case EDBAZodiac::Tiger: return FLinearColor(1.00f, 0.26f, 0.04f, 1.0f);
	case EDBAZodiac::Rabbit: return FLinearColor(0.22f, 0.94f, 0.52f, 1.0f);
	case EDBAZodiac::Dragon: return FLinearColor(1.00f, 0.78f, 0.02f, 1.0f);
	case EDBAZodiac::Snake: return FLinearColor(0.04f, 0.78f, 0.32f, 1.0f);
	case EDBAZodiac::Horse: return FLinearColor(1.00f, 0.08f, 0.04f, 1.0f);
	case EDBAZodiac::Goat: return FLinearColor(0.92f, 0.74f, 0.38f, 1.0f);
	case EDBAZodiac::Monkey: return FLinearColor(0.08f, 0.82f, 1.00f, 1.0f);
	case EDBAZodiac::Rooster: return FLinearColor(1.00f, 0.18f, 0.06f, 1.0f);
	case EDBAZodiac::Dog: return FLinearColor(0.10f, 0.34f, 1.00f, 1.0f);
	case EDBAZodiac::Pig: return FLinearColor(1.00f, 0.34f, 0.54f, 1.0f);
	default: return FLinearColor(0.18f, 0.48f, 1.00f, 1.0f);
	}
}
