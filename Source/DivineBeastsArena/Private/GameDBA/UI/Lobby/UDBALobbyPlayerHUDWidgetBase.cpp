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
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Data/DBAFixedSkillGroupData.h"
#include "GameDBA/Data/DBASkillDataRow.h"
#include "GameDBA/GAS/DBAAbilitySetLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Styling/CoreStyle.h"

namespace
{
	constexpr int32 LobbySkillSlotCount = 7;
	const TCHAR* DesktopSkillHotkeys[LobbySkillSlotCount] = { TEXT("AUTO"), TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("AUTO") };
	const TCHAR* MobileSkillHotkeys[LobbySkillSlotCount] = { TEXT("AUTO"), TEXT("S1"), TEXT("S2"), TEXT("S3"), TEXT("S4"), TEXT("ULT"), TEXT("AUTO") };

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

	int32 SkillSlotToLobbyIndex(EDBASkillSlot Slot)
	{
		switch (Slot)
		{
		case EDBASkillSlot::Passive: return 0;
		case EDBASkillSlot::Active1: return 1;
		case EDBASkillSlot::Active2: return 2;
		case EDBASkillSlot::Active3: return 3;
		case EDBASkillSlot::Active4: return 4;
		case EDBASkillSlot::Ultimate: return 5;
		default: return INDEX_NONE;
		}
	}

	FString ZodiacSkillFallbackName(EDBAZodiac Zodiac, int32 SkillIndex)
	{
		static const TCHAR* SlotNames[LobbySkillSlotCount] = {
			TEXT("Passive"),
			TEXT("Skill 1"),
			TEXT("Skill 2"),
			TEXT("Skill 3"),
			TEXT("Skill 4"),
			TEXT("Ultimate"),
			TEXT("Resonance")
		};

		const UEnum* ZodiacEnum = StaticEnum<EDBAZodiac>();
		FString ZodiacName = ZodiacEnum ? ZodiacEnum->GetNameStringByValue(static_cast<int64>(Zodiac)) : TEXT("Zodiac");
		if (ZodiacName.IsEmpty() || Zodiac == EDBAZodiac::None)
		{
			ZodiacName = TEXT("Zodiac");
		}

		const TCHAR* SlotName = (SkillIndex >= 0 && SkillIndex < LobbySkillSlotCount) ? SlotNames[SkillIndex] : TEXT("Skill");
		return FString::Printf(TEXT("%s %s"), *ZodiacName, SlotName);
	}

	FText KeyToSkillHotkeyText(const FKey& Key, const TCHAR* Fallback)
	{
		if (!Key.IsValid())
		{
			return FText::FromString(Fallback ? FString(Fallback) : FString(TEXT("AUTO")));
		}

		const FText DisplayName = Key.GetDisplayName(false);
		return DisplayName.IsEmpty() ? FText::FromName(Key.GetFName()) : DisplayName;
	}

	void ApplyFixedSkillGroupAssetHotkeys(const UDBAFixedSkillGroupDataAsset* SkillGroupAsset, TArray<FText>& OutSkillHotkeys)
	{
		if (!SkillGroupAsset)
		{
			return;
		}

		OutSkillHotkeys = {
			KeyToSkillHotkeyText(SkillGroupAsset->PassiveInputKey, DesktopSkillHotkeys[0]),
			KeyToSkillHotkeyText(SkillGroupAsset->Skill01InputKey, DesktopSkillHotkeys[1]),
			KeyToSkillHotkeyText(SkillGroupAsset->Skill02InputKey, DesktopSkillHotkeys[2]),
			KeyToSkillHotkeyText(SkillGroupAsset->Skill03InputKey, DesktopSkillHotkeys[3]),
			KeyToSkillHotkeyText(SkillGroupAsset->Skill04InputKey, DesktopSkillHotkeys[4]),
			KeyToSkillHotkeyText(SkillGroupAsset->ZodiacUltimateInputKey, DesktopSkillHotkeys[5]),
			KeyToSkillHotkeyText(SkillGroupAsset->ResonanceInputKey, DesktopSkillHotkeys[6])
		};
	}

	void ApplyFixedSkillGroupRowHotkeys(const FDBAZodiacElementFixedSkillGroupRow& SkillGroupRow, TArray<FText>& OutSkillHotkeys)
	{
		OutSkillHotkeys = {
			KeyToSkillHotkeyText(SkillGroupRow.ElementPassiveInputKey, DesktopSkillHotkeys[0]),
			KeyToSkillHotkeyText(SkillGroupRow.ElementSkill1InputKey, DesktopSkillHotkeys[1]),
			KeyToSkillHotkeyText(SkillGroupRow.ElementSkill2InputKey, DesktopSkillHotkeys[2]),
			KeyToSkillHotkeyText(SkillGroupRow.ElementSkill3InputKey, DesktopSkillHotkeys[3]),
			KeyToSkillHotkeyText(SkillGroupRow.ElementSkill4InputKey, DesktopSkillHotkeys[4]),
			KeyToSkillHotkeyText(SkillGroupRow.ZodiacUltimateInputKey, DesktopSkillHotkeys[5]),
			KeyToSkillHotkeyText(SkillGroupRow.ResonanceInputKey, DesktopSkillHotkeys[6])
		};
	}

	EDBAZodiac ToCommonZodiac(EDBAZodiacType ZodiacType)
	{
		switch (ZodiacType)
		{
		case EDBAZodiacType::Rat: return EDBAZodiac::Rat;
		case EDBAZodiacType::Ox: return EDBAZodiac::Ox;
		case EDBAZodiacType::Tiger: return EDBAZodiac::Tiger;
		case EDBAZodiacType::Rabbit: return EDBAZodiac::Rabbit;
		case EDBAZodiacType::Dragon: return EDBAZodiac::Dragon;
		case EDBAZodiacType::Snake: return EDBAZodiac::Snake;
		case EDBAZodiacType::Horse: return EDBAZodiac::Horse;
		case EDBAZodiacType::Goat: return EDBAZodiac::Goat;
		case EDBAZodiacType::Monkey: return EDBAZodiac::Monkey;
		case EDBAZodiacType::Rooster: return EDBAZodiac::Rooster;
		case EDBAZodiacType::Dog: return EDBAZodiac::Dog;
		case EDBAZodiacType::Pig: return EDBAZodiac::Pig;
		default: return EDBAZodiac::None;
		}
	}

	EDBAZodiac ResolvePawnZodiac(const UUserWidget* Widget)
	{
		const APlayerController* PC = Widget ? Widget->GetOwningPlayer() : nullptr;
		const ADBAZodiacCharacterBase* Character = PC ? Cast<ADBAZodiacCharacterBase>(PC->GetPawn()) : nullptr;
		return Character ? ToCommonZodiac(Character->GetZodiacType()) : EDBAZodiac::None;
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
	BuildDefaultLayoutIfNeeded();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	RefreshFromCurrentCharacterData();
	UE_LOG(LogDBACore, Log, TEXT("[LobbyPlayerHUD] Constructed: Avatar=%s Skills=%d Minimap=%s Viewport=%s"),
		AvatarRootBorder ? TEXT("true") : TEXT("false"),
		SkillSlotBorders.Num(),
		MinimapRootBorder ? TEXT("true") : TEXT("false"),
		*UWidgetLayoutLibrary::GetViewportSize(this).ToString());
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
		const EDBAZodiac PawnZodiac = ResolvePawnZodiac(this);
		if (AvatarNameText)
		{
			AvatarNameText->SetText(FText::FromString(TEXT("Player")));
		}
		if (AvatarMetaText)
		{
			AvatarMetaText->SetText(PawnZodiac == EDBAZodiac::None
				? FText::FromString(TEXT("No Character Data"))
				: FText::Format(FText::FromString(TEXT("{0} | Lobby | Lv.1")), ZodiacToShortText(PawnZodiac)));
		}
		if (AvatarImage && PawnZodiac != EDBAZodiac::None)
		{
			AvatarImage->SetColorAndOpacity(ZodiacToColor(PawnZodiac));
		}
		Summary.Zodiac = PawnZodiac;
		Summary.DefaultZodiac = PawnZodiac;
		ApplySkillLabels({
			FText::FromString(ZodiacSkillFallbackName(PawnZodiac, 0)),
			FText::FromString(ZodiacSkillFallbackName(PawnZodiac, 1)),
			FText::FromString(ZodiacSkillFallbackName(PawnZodiac, 2)),
			FText::FromString(ZodiacSkillFallbackName(PawnZodiac, 3)),
			FText::FromString(ZodiacSkillFallbackName(PawnZodiac, 4)),
			FText::FromString(ZodiacSkillFallbackName(PawnZodiac, 5)),
			FText::FromString(ZodiacSkillFallbackName(PawnZodiac, 6))});
		ApplySkillHotkeys({
			FText::FromString(DesktopSkillHotkeys[0]),
			FText::FromString(DesktopSkillHotkeys[1]),
			FText::FromString(DesktopSkillHotkeys[2]),
			FText::FromString(DesktopSkillHotkeys[3]),
			FText::FromString(DesktopSkillHotkeys[4]),
			FText::FromString(DesktopSkillHotkeys[5]),
			FText::FromString(DesktopSkillHotkeys[6])});
		if (PawnZodiac != EDBAZodiac::None)
		{
			TArray<FText> SkillLabels;
			ResolveSkillLabelsForSummary(Summary, SkillLabels);
			ApplySkillLabels(SkillLabels);
			TArray<FText> SkillHotkeys;
			ResolveSkillHotkeysForSummary(Summary, SkillHotkeys);
			ApplySkillHotkeys(SkillHotkeys);
		}
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
	TArray<FText> SkillHotkeys;
	ResolveSkillHotkeysForSummary(Summary, SkillHotkeys);
	ApplySkillHotkeys(SkillHotkeys);
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

	const TCHAR* const* Hotkeys = bMobile ? MobileSkillHotkeys : DesktopSkillHotkeys;
	SkillSlotBorders.Reset();
	SkillNameTexts.Reset();
	SkillHotkeyTexts.Reset();
	SkillCooldownTexts.Reset();

	for (int32 Index = 0; Index < LobbySkillSlotCount; ++Index)
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
		NameText->SetText(FText::FromString(ZodiacSkillFallbackName(EDBAZodiac::None, Index)));
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
			HotkeyText->SetText(FText::FromString(bMobileLike ? MobileSkillHotkeys[Index] : DesktopSkillHotkeys[Index]));
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

void UDBALobbyPlayerHUDWidgetBase::ApplySkillHotkeys(const TArray<FText>& SkillHotkeys)
{
	const bool bMobile = IsMobilePlatform();
	for (int32 Index = 0; Index < SkillHotkeyTexts.Num(); ++Index)
	{
		if (UTextBlock* HotkeyText = SkillHotkeyTexts[Index])
		{
			const FText Fallback = FText::FromString(bMobile ? MobileSkillHotkeys[Index] : DesktopSkillHotkeys[Index]);
			HotkeyText->SetText(SkillHotkeys.IsValidIndex(Index) ? SkillHotkeys[Index] : Fallback);
		}
	}
}

void UDBALobbyPlayerHUDWidgetBase::ResolveSkillLabelsForSummary(const FDBACharacterSummary& Summary, TArray<FText>& OutSkillLabels) const
{
	const EDBAZodiac Zodiac = Summary.Zodiac == EDBAZodiac::None ? Summary.DefaultZodiac : Summary.Zodiac;
	OutSkillLabels = {
		FText::FromString(ZodiacSkillFallbackName(Zodiac, 0)),
		FText::FromString(ZodiacSkillFallbackName(Zodiac, 1)),
		FText::FromString(ZodiacSkillFallbackName(Zodiac, 2)),
		FText::FromString(ZodiacSkillFallbackName(Zodiac, 3)),
		FText::FromString(ZodiacSkillFallbackName(Zodiac, 4)),
		FText::FromString(ZodiacSkillFallbackName(Zodiac, 5)),
		FText::FromString(ZodiacSkillFallbackName(Zodiac, 6))
	};

	if (Zodiac == EDBAZodiac::None)
	{
		return;
	}

	UDataTable* SkillDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/Skills/SkillDataTable.SkillDataTable"));
	if (!SkillDataTable)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[LobbyPlayerHUD] SkillDataTable unavailable for zodiac=%d"), static_cast<int32>(Zodiac));
		return;
	}

	TArray<FDBASkillDataRow*> Rows;
	SkillDataTable->GetAllRows<FDBASkillDataRow>(TEXT("LobbyHUD"), Rows);
	int32 AppliedSkillCount = 0;
	for (const FDBASkillDataRow* Row : Rows)
	{
		if (!Row || Row->ZodiacType != Zodiac || !Row->bEnabled || Row->bIsInDevelopment)
		{
			continue;
		}

		const int32 SkillIndex = SkillSlotToLobbyIndex(Row->SkillSlot);
		if (!OutSkillLabels.IsValidIndex(SkillIndex))
		{
			continue;
		}

		OutSkillLabels[SkillIndex] = Row->DisplayName.IsEmpty() ? FText::FromName(Row->SkillId) : Row->DisplayName;
		++AppliedSkillCount;
	}

	UE_LOG(LogDBACore, Log, TEXT("[LobbyPlayerHUD] Loaded zodiac skill labels: zodiac=%d applied=%d rows=%d"),
		static_cast<int32>(Zodiac),
		AppliedSkillCount,
		Rows.Num());
}

void UDBALobbyPlayerHUDWidgetBase::ResolveSkillHotkeysForSummary(const FDBACharacterSummary& Summary, TArray<FText>& OutSkillHotkeys) const
{
	const bool bMobile = IsMobilePlatform();
	OutSkillHotkeys = {
		FText::FromString(bMobile ? MobileSkillHotkeys[0] : DesktopSkillHotkeys[0]),
		FText::FromString(bMobile ? MobileSkillHotkeys[1] : DesktopSkillHotkeys[1]),
		FText::FromString(bMobile ? MobileSkillHotkeys[2] : DesktopSkillHotkeys[2]),
		FText::FromString(bMobile ? MobileSkillHotkeys[3] : DesktopSkillHotkeys[3]),
		FText::FromString(bMobile ? MobileSkillHotkeys[4] : DesktopSkillHotkeys[4]),
		FText::FromString(bMobile ? MobileSkillHotkeys[5] : DesktopSkillHotkeys[5]),
		FText::FromString(bMobile ? MobileSkillHotkeys[6] : DesktopSkillHotkeys[6])
	};

	if (bMobile || Summary.FixedSkillGroupId.IsNone())
	{
		return;
	}

	if (const UDBAFixedSkillGroupDataAsset* SkillGroupAsset = UDBAFixedSkillGroupLibrary::GetFixedSkillGroupById(Summary.FixedSkillGroupId))
	{
		ApplyFixedSkillGroupAssetHotkeys(SkillGroupAsset, OutSkillHotkeys);
		UE_LOG(LogDBACore, Log, TEXT("[LobbyPlayerHUD] Loaded skill hotkeys from FixedSkillGroup asset: %s"),
			*Summary.FixedSkillGroupId.ToString());
		return;
	}

	UDataTable* FixedSkillGroupTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/DBA/Data/Tables/DT_FixedSkillGroups.DT_FixedSkillGroups"));
	if (!FixedSkillGroupTable)
	{
		return;
	}

	if (const FDBAZodiacElementFixedSkillGroupRow* Row = FixedSkillGroupTable->FindRow<FDBAZodiacElementFixedSkillGroupRow>(Summary.FixedSkillGroupId, TEXT("LobbyHUD")))
	{
		ApplyFixedSkillGroupRowHotkeys(*Row, OutSkillHotkeys);
		UE_LOG(LogDBACore, Log, TEXT("[LobbyPlayerHUD] Loaded skill hotkeys from FixedSkillGroup table row: %s"),
			*Summary.FixedSkillGroupId.ToString());
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
