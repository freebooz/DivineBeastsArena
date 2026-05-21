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
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
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
	const FVector2D LobbyAvatarPanelLimit(190.0f, 58.0f);
	const FVector2D LobbySkillSlotLimit(32.0f, 32.0f);
	const FVector2D LobbySkillBarLimit(258.0f, 40.0f);
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

	UTexture2D* LoadHudTexture(const TCHAR* ObjectPath)
	{
		return ObjectPath ? LoadObject<UTexture2D>(nullptr, ObjectPath) : nullptr;
	}

	void ApplyTextureBrush(UImage* Image, UTexture2D* Texture, const FLinearColor& Tint)
	{
		if (!Image)
		{
			return;
		}

		if (Texture)
		{
			Image->SetBrushFromTexture(Texture, true);
		}
		Image->SetColorAndOpacity(Tint);
	}

	FLinearColor SkillGemColor(int32 Index)
	{
		static const FLinearColor Colors[LobbySkillSlotCount] = {
			FLinearColor(0.58f, 0.44f, 0.22f, 0.92f),
			FLinearColor(0.95f, 0.22f, 0.06f, 0.96f),
			FLinearColor(0.12f, 0.38f, 0.90f, 0.92f),
			FLinearColor(0.12f, 0.78f, 0.42f, 0.92f),
			FLinearColor(0.90f, 0.58f, 0.12f, 0.92f),
			FLinearColor(0.62f, 0.18f, 0.98f, 0.96f),
			FLinearColor(0.15f, 0.85f, 0.95f, 0.92f)
		};
		return Colors[FMath::Clamp(Index, 0, LobbySkillSlotCount - 1)];
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
	EnforceLobbyHudLayoutLimits();
	BuildDefaultLayoutIfNeeded();
}

void UDBALobbyPlayerHUDWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	EnforceLobbyHudLayoutLimits();
	BuildDefaultLayoutIfNeeded();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	RefreshFromCurrentCharacterData();
	UE_LOG(LogDBACore, Log, TEXT("[LobbyPlayerHUD] Constructed: Avatar=%s Skills=%d Minimap=%s Viewport=%s AvatarSize=%s SkillSlotSize=%s SkillBarSize=%s"),
		AvatarRootBorder ? TEXT("true") : TEXT("false"),
		SkillSlotBorders.Num(),
		MinimapRootBorder ? TEXT("true") : TEXT("false"),
		*UWidgetLayoutLibrary::GetViewportSize(this).ToString(),
		*AvatarPanelSize.ToString(),
		*SkillSlotSize.ToString(),
		*LobbySkillBarLimit.ToString());
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
	UpdateSkillCooldownDisplay(InDeltaTime);
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
	EnforceLobbyHudLayoutLimits();

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

void UDBALobbyPlayerHUDWidgetBase::EnforceLobbyHudLayoutLimits()
{
	AvatarPanelSize = LobbyAvatarPanelLimit;
	SkillSlotSize = LobbySkillSlotLimit;

	if (AvatarRootSlot)
	{
		AvatarRootSlot->SetAutoSize(false);
		AvatarRootSlot->SetSize(AvatarPanelSize);
	}

	if (SkillBarRootSlot)
	{
		SkillBarRootSlot->SetAutoSize(false);
		SkillBarRootSlot->SetSize(LobbySkillBarLimit);
	}

	if (AvatarRootBorder)
	{
		AvatarRootBorder->SetClipping(EWidgetClipping::ClipToBounds);
		AvatarRootBorder->SetDesiredSizeScale(FVector2D(1.0f, 1.0f));
	}

	if (SkillBarRootBorder)
	{
		SkillBarRootBorder->SetClipping(EWidgetClipping::ClipToBounds);
		SkillBarRootBorder->SetDesiredSizeScale(FVector2D(1.0f, 1.0f));
	}

	for (UBorder* SkillSlotBorder : SkillSlotBorders)
	{
		if (SkillSlotBorder)
		{
			SkillSlotBorder->SetClipping(EWidgetClipping::ClipToBounds);
			SkillSlotBorder->SetDesiredSizeScale(FVector2D(1.0f, 1.0f));
			SkillSlotBorder->SetRenderScale(FVector2D(1.0f, 1.0f));
		}
	}
}

void UDBALobbyPlayerHUDWidgetBase::BuildTopLeftAvatarPanel(UCanvasPanel* RootCanvas)
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	const bool bMobile = IsMobilePlatform();
	const FVector2D LocalPanelSize = bMobile ? AvatarPanelSize * 0.88f : AvatarPanelSize;

	AvatarRootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyHUD_AvatarRoot"));
	AvatarRootBorder->SetBrushColor(FLinearColor(0.01f, 0.008f, 0.006f, 0.92f));
	AvatarRootBorder->SetPadding(FMargin(4.0f));
	AvatarRootBorder->SetClipping(EWidgetClipping::ClipToBounds);

	UCanvasPanelSlot* RootSlot = RootCanvas->AddChildToCanvas(AvatarRootBorder);
	AvatarRootSlot = RootSlot;
	RootSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	RootSlot->SetAlignment(FVector2D::ZeroVector);
	RootSlot->SetPosition(FVector2D(18.0f, 18.0f));
	RootSlot->SetAutoSize(false);
	RootSlot->SetSize(LocalPanelSize);
	RootSlot->SetZOrder(40);

	UOverlay* AvatarOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("LobbyHUD_AvatarOverlay"));
	AvatarRootBorder->SetContent(AvatarOverlay);

	AvatarBackdropImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("LobbyHUD_AvatarBackdrop"));
	ApplyTextureBrush(
		AvatarBackdropImage,
		LoadHudTexture(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginPanel_StoneGold.T_DBA_LoginPanel_StoneGold")),
		FLinearColor(0.28f, 0.22f, 0.16f, 0.92f));
	if (UOverlaySlot* BackdropSlot = AvatarOverlay->AddChildToOverlay(AvatarBackdropImage))
	{
		BackdropSlot->SetHorizontalAlignment(HAlign_Fill);
		BackdropSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UHorizontalBox* RootHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("LobbyHUD_AvatarHBox"));
	if (UOverlaySlot* ContentSlot = AvatarOverlay->AddChildToOverlay(RootHBox))
	{
		ContentSlot->SetHorizontalAlignment(HAlign_Fill);
		ContentSlot->SetVerticalAlignment(VAlign_Fill);
		ContentSlot->SetPadding(FMargin(4.0f));
	}

	UOverlay* PortraitOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("LobbyHUD_PortraitOverlay"));
	if (UHorizontalBoxSlot* ImageSlot = RootHBox->AddChildToHorizontalBox(PortraitOverlay))
	{
		const float SquareSize = FMath::Clamp(LocalPanelSize.Y - 12.0f, 38.0f, 46.0f);
		ImageSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		ImageSlot->SetPadding(FMargin(0.0f, 0.0f, 5.0f, 0.0f));
		PortraitOverlay->SetClipping(EWidgetClipping::ClipToBounds);
	}

	AvatarImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("LobbyHUD_AvatarImage"));
	ApplyTextureBrush(
		AvatarImage,
		LoadHudTexture(TEXT("/Game/DBA/Characters/Rosales/Meshes/T_Rosales_Diffuse.T_Rosales_Diffuse")),
		FLinearColor(0.18f, 0.48f, 1.0f, 1.0f));
	const float PortraitSize = FMath::Clamp(LocalPanelSize.Y - 12.0f, 38.0f, 46.0f);
	AvatarImage->SetDesiredSizeOverride(FVector2D(PortraitSize, PortraitSize));
	if (UOverlaySlot* PortraitImageSlot = PortraitOverlay->AddChildToOverlay(AvatarImage))
	{
		PortraitImageSlot->SetHorizontalAlignment(HAlign_Fill);
		PortraitImageSlot->SetVerticalAlignment(VAlign_Fill);
		PortraitImageSlot->SetPadding(FMargin(5.0f));
	}

	AvatarFrameImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("LobbyHUD_AvatarFrame"));
	ApplyTextureBrush(
		AvatarFrameImage,
		LoadHudTexture(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginButton_ParchmentGold.T_DBA_LoginButton_ParchmentGold")),
		FLinearColor(1.0f, 0.76f, 0.24f, 0.96f));
	AvatarFrameImage->SetDesiredSizeOverride(FVector2D(PortraitSize, PortraitSize));
	if (UOverlaySlot* PortraitFrameSlot = PortraitOverlay->AddChildToOverlay(AvatarFrameImage))
	{
		PortraitFrameSlot->SetHorizontalAlignment(HAlign_Fill);
		PortraitFrameSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UVerticalBox* MetaVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LobbyHUD_AvatarMetaVBox"));
	if (UHorizontalBoxSlot* VBoxSlot = RootHBox->AddChildToHorizontalBox(MetaVBox))
	{
		VBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		VBoxSlot->SetHorizontalAlignment(HAlign_Left);
		VBoxSlot->SetVerticalAlignment(VAlign_Center);
	}

	AvatarNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyHUD_AvatarNameText"));
	AvatarNameText->SetText(FText::FromString(TEXT("Player")));
	AvatarNameText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), bMobile ? 12 : 13));
	AvatarNameText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.88f, 0.56f, 1.0f)));
	AvatarNameText->SetAutoWrapText(false);
	AvatarNameText->SetClipping(EWidgetClipping::ClipToBounds);
	MetaVBox->AddChildToVerticalBox(AvatarNameText);

	AvatarMetaText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LobbyHUD_AvatarMetaText"));
	AvatarMetaText->SetText(FText::FromString(TEXT("Zodiac | Element | Lv.1")));
	AvatarMetaText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), bMobile ? 9 : 10));
	AvatarMetaText->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 0.84f, 0.9f, 1.0f)));
	AvatarMetaText->SetAutoWrapText(false);
	AvatarMetaText->SetClipping(EWidgetClipping::ClipToBounds);
	MetaVBox->AddChildToVerticalBox(AvatarMetaText);
}

void UDBALobbyPlayerHUDWidgetBase::BuildBottomSkillBar(UCanvasPanel* RootCanvas)
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	const bool bMobile = IsMobilePlatform();
	const FVector2D LocalSlotSize = bMobile ? SkillSlotSize * 0.94f : SkillSlotSize;

	SkillBarRootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LobbyHUD_SkillBarRoot"));
	SkillBarRootBorder->SetBrushColor(FLinearColor(0.01f, 0.008f, 0.006f, 0.88f));
	SkillBarRootBorder->SetPadding(FMargin(3.0f, 2.0f));
	SkillBarRootBorder->SetClipping(EWidgetClipping::ClipToBounds);
	UCanvasPanelSlot* SkillBarSlot = RootCanvas->AddChildToCanvas(SkillBarRootBorder);
	SkillBarRootSlot = SkillBarSlot;
	SkillBarSlot->SetAnchors(FAnchors(0.5f, 1.0f));
	SkillBarSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	SkillBarSlot->SetPosition(FVector2D(0.0f, bMobile ? -10.0f : -18.0f));
	SkillBarSlot->SetAutoSize(false);
	SkillBarSlot->SetSize(bMobile ? LobbySkillBarLimit * 0.94f : LobbySkillBarLimit);
	SkillBarSlot->SetZOrder(45);

	UOverlay* SkillBarOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("LobbyHUD_SkillBarOverlay"));
	SkillBarRootBorder->SetContent(SkillBarOverlay);

	SkillBarBackdropImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("LobbyHUD_SkillBarBackdrop"));
	ApplyTextureBrush(
		SkillBarBackdropImage,
		LoadHudTexture(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginPanel_StoneGold.T_DBA_LoginPanel_StoneGold")),
		FLinearColor(0.22f, 0.17f, 0.11f, 0.88f));
	SkillBarBackdropImage->SetDesiredSizeOverride(bMobile ? LobbySkillBarLimit * 0.94f : LobbySkillBarLimit);
	SkillBarBackdropImage->SetClipping(EWidgetClipping::ClipToBounds);
	if (UOverlaySlot* BackdropSlot = SkillBarOverlay->AddChildToOverlay(SkillBarBackdropImage))
	{
		BackdropSlot->SetHorizontalAlignment(HAlign_Fill);
		BackdropSlot->SetVerticalAlignment(VAlign_Fill);
		BackdropSlot->SetPadding(FMargin(0.0f));
	}

	UHorizontalBox* SkillBarHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("LobbyHUD_SkillBarHBox"));
	if (UOverlaySlot* BarContentSlot = SkillBarOverlay->AddChildToOverlay(SkillBarHBox))
	{
		BarContentSlot->SetHorizontalAlignment(HAlign_Fill);
		BarContentSlot->SetVerticalAlignment(VAlign_Fill);
		BarContentSlot->SetPadding(FMargin(2.0f, 1.0f));
	}

	const TCHAR* const* Hotkeys = bMobile ? MobileSkillHotkeys : DesktopSkillHotkeys;
	SkillSlotBorders.Reset();
	SkillSlotBackdropImages.Reset();
	SkillCooldownOverlayImages.Reset();
	SkillReadyGlowImages.Reset();
	SkillNameTexts.Reset();
	SkillHotkeyTexts.Reset();
	SkillCooldownTexts.Reset();
	LastObservedSkillCooldowns.Init(0.0f, LobbySkillSlotCount);
	SkillReadyPulseTimes.Init(0.0f, LobbySkillSlotCount);

	for (int32 Index = 0; Index < LobbySkillSlotCount; ++Index)
	{
		UBorder* SlotBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillSlot_%d"), Index));
		SlotBorder->SetBrushColor(FLinearColor(0.02f, 0.018f, 0.014f, 0.94f));
		SlotBorder->SetPadding(FMargin(2.0f));
		SlotBorder->SetClipping(EWidgetClipping::ClipToBounds);

		USizeBox* SlotSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillSize_%d"), Index));
		SlotSizeBox->SetWidthOverride(LocalSlotSize.X);
		SlotSizeBox->SetHeightOverride(LocalSlotSize.Y);
		SlotSizeBox->AddChild(SlotBorder);

		if (UHorizontalBoxSlot* SkillHBoxSlot = SkillBarHBox->AddChildToHorizontalBox(SlotSizeBox))
		{
			SkillHBoxSlot->SetPadding(FMargin(1.0f, 0.0f));
			SkillHBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		}

		UOverlay* SlotOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillOverlay_%d"), Index));
		SlotBorder->SetContent(SlotOverlay);

		UImage* SlotBackdrop = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillBack_%d"), Index));
		ApplyTextureBrush(
			SlotBackdrop,
			LoadHudTexture(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginButton_ParchmentGold.T_DBA_LoginButton_ParchmentGold")),
			SkillGemColor(Index));
		SlotBackdrop->SetDesiredSizeOverride(LocalSlotSize);
		SlotBackdrop->SetClipping(EWidgetClipping::ClipToBounds);
		if (UOverlaySlot* SlotBackOverlay = SlotOverlay->AddChildToOverlay(SlotBackdrop))
		{
			SlotBackOverlay->SetHorizontalAlignment(HAlign_Fill);
			SlotBackOverlay->SetVerticalAlignment(VAlign_Fill);
		}

		UImage* ReadyGlow = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillGlow_%d"), Index));
		ReadyGlow->SetColorAndOpacity(FLinearColor(1.0f, 0.78f, 0.22f, 0.18f));
		ReadyGlow->SetVisibility(ESlateVisibility::Visible);
		if (UOverlaySlot* GlowSlot = SlotOverlay->AddChildToOverlay(ReadyGlow))
		{
			GlowSlot->SetHorizontalAlignment(HAlign_Fill);
			GlowSlot->SetVerticalAlignment(VAlign_Fill);
			GlowSlot->SetPadding(FMargin(2.0f));
		}

		UVerticalBox* ContentVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillVBox_%d"), Index));
		if (UOverlaySlot* OverlaySlot = SlotOverlay->AddChildToOverlay(ContentVBox))
		{
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			OverlaySlot->SetVerticalAlignment(VAlign_Fill);
			OverlaySlot->SetPadding(FMargin(2.0f));
		}

		UTextBlock* NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillName_%d"), Index));
		NameText->SetText(FText::FromString(ZodiacSkillFallbackName(EDBAZodiac::None, Index)));
		NameText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), bMobile ? 9 : 10));
		NameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.95f, 0.98f, 1.0f)));
		NameText->SetVisibility(ESlateVisibility::Collapsed);

		UTextBlock* CooldownText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillCD_%d"), Index));
		CooldownText->SetText(FText::GetEmpty());
		CooldownText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), bMobile ? 11 : 12));
		CooldownText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.76f, 0.28f, 1.0f)));
		if (UOverlaySlot* CooldownTextSlot = SlotOverlay->AddChildToOverlay(CooldownText))
		{
			CooldownTextSlot->SetHorizontalAlignment(HAlign_Center);
			CooldownTextSlot->SetVerticalAlignment(VAlign_Center);
		}

		UImage* CooldownOverlay = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillCooldownOverlay_%d"), Index));
		CooldownOverlay->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.68f));
		CooldownOverlay->SetRenderTransformPivot(FVector2D(0.5f, 1.0f));
		CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
		if (UOverlaySlot* CooldownOverlaySlot = SlotOverlay->AddChildToOverlay(CooldownOverlay))
		{
			CooldownOverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			CooldownOverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}

		UTextBlock* HotkeyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("LobbyHUD_SkillHotkey_%d"), Index));
		HotkeyText->SetText(FText::FromString(Hotkeys[Index]));
		HotkeyText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), bMobile ? 7 : 8));
		HotkeyText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.84f, 0.18f, 1.0f)));
		if (UOverlaySlot* KeyOverlaySlot = SlotOverlay->AddChildToOverlay(HotkeyText))
		{
			KeyOverlaySlot->SetHorizontalAlignment(HAlign_Right);
			KeyOverlaySlot->SetVerticalAlignment(VAlign_Bottom);
			KeyOverlaySlot->SetPadding(FMargin(0.0f, 0.0f, 2.0f, 1.0f));
		}

		SlotBorder->SetDesiredSizeScale(FVector2D(1.0f, 1.0f));

		SkillSlotBorders.Add(SlotBorder);
		SkillSlotBackdropImages.Add(SlotBackdrop);
		SkillCooldownOverlayImages.Add(CooldownOverlay);
		SkillReadyGlowImages.Add(ReadyGlow);
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

void UDBALobbyPlayerHUDWidgetBase::UpdateSkillCooldownDisplay(float DeltaTime)
{
	const APlayerController* PC = GetOwningPlayer();
	const ADBAZodiacCharacterBase* Character = PC ? Cast<ADBAZodiacCharacterBase>(PC->GetPawn()) : nullptr;
	const TArray<float> Cooldowns = Character ? Character->GetSkillCooldowns() : TArray<float>();
	const TArray<float> MaxCooldowns = Character ? Character->GetSkillMaxCooldowns() : TArray<float>();

	if (LastObservedSkillCooldowns.Num() != LobbySkillSlotCount)
	{
		LastObservedSkillCooldowns.Init(0.0f, LobbySkillSlotCount);
	}
	if (SkillReadyPulseTimes.Num() != LobbySkillSlotCount)
	{
		SkillReadyPulseTimes.Init(0.0f, LobbySkillSlotCount);
	}

	for (int32 Index = 0; Index < LobbySkillSlotCount; ++Index)
	{
		const float Remaining = Cooldowns.IsValidIndex(Index) ? FMath::Max(0.0f, Cooldowns[Index]) : 0.0f;
		const float Total = MaxCooldowns.IsValidIndex(Index) ? FMath::Max(0.0f, MaxCooldowns[Index]) : 0.0f;
		const float Previous = LastObservedSkillCooldowns[Index];
		if (Previous > 0.0f && Remaining <= 0.0f)
		{
			SkillReadyPulseTimes[Index] = 0.28f;
		}
		LastObservedSkillCooldowns[Index] = Remaining;

		if (UImage* Overlay = SkillCooldownOverlayImages.IsValidIndex(Index) ? SkillCooldownOverlayImages[Index] : nullptr)
		{
			if (Remaining > 0.0f && Total > 0.0f)
			{
				const float Percent = FMath::Clamp(Remaining / Total, 0.0f, 1.0f);
				Overlay->SetVisibility(ESlateVisibility::Visible);
				Overlay->SetRenderScale(FVector2D(1.0f, Percent));
				Overlay->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.70f));
			}
			else
			{
				Overlay->SetVisibility(ESlateVisibility::Hidden);
				Overlay->SetRenderScale(FVector2D(1.0f, 0.0f));
			}
		}

		if (UTextBlock* CooldownText = SkillCooldownTexts.IsValidIndex(Index) ? SkillCooldownTexts[Index] : nullptr)
		{
			if (Remaining > 0.0f)
			{
				CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(Remaining)));
				CooldownText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.76f, 0.28f, 1.0f)));
			}
			else
			{
				CooldownText->SetText(FText::GetEmpty());
				CooldownText->SetColorAndOpacity(FSlateColor(FLinearColor(0.52f, 0.95f, 0.72f, 1.0f)));
			}
		}

		const float Pulse = FMath::Max(0.0f, SkillReadyPulseTimes[Index] - DeltaTime);
		SkillReadyPulseTimes[Index] = Pulse;
		const bool bCoolingDown = Remaining > 0.0f;
		const float PulseAlpha = Pulse > 0.0f ? FMath::Clamp(Pulse / 0.28f, 0.0f, 1.0f) : 0.0f;
		if (UImage* ReadyGlow = SkillReadyGlowImages.IsValidIndex(Index) ? SkillReadyGlowImages[Index] : nullptr)
		{
			ReadyGlow->SetColorAndOpacity(bCoolingDown
				? FLinearColor(0.02f, 0.02f, 0.02f, 0.12f)
				: FLinearColor(1.0f, 0.76f, 0.22f, 0.14f + PulseAlpha * 0.42f));
			ReadyGlow->SetRenderScale(FVector2D(1.0f + PulseAlpha * 0.08f, 1.0f + PulseAlpha * 0.08f));
		}

		if (UBorder* SlotBorder = SkillSlotBorders.IsValidIndex(Index) ? SkillSlotBorders[Index] : nullptr)
		{
			const float CooldownDim = bCoolingDown ? 0.72f : 1.0f;
			SlotBorder->SetRenderScale(FVector2D(1.0f, 1.0f));
			if (UImage* SlotBack = SkillSlotBackdropImages.IsValidIndex(Index) ? SkillSlotBackdropImages[Index] : nullptr)
			{
				FLinearColor Color = SkillGemColor(Index);
				Color.R *= CooldownDim;
				Color.G *= CooldownDim;
				Color.B *= CooldownDim;
				SlotBack->SetColorAndOpacity(Color);
			}
		}
	}
}

void UDBALobbyPlayerHUDWidgetBase::ApplyResponsiveLayout(const FVector2D& ViewportSize)
{
	EnforceLobbyHudLayoutLimits();

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
		AvatarRootSlot->SetAutoSize(false);
		AvatarRootSlot->SetSize(AvatarPanelSize * BaseScale);
	}

	if (SkillBarRootSlot)
	{
		SkillBarRootSlot->SetPosition(FVector2D(0.0f, bMobileLike ? -8.0f : -18.0f));
		SkillBarRootSlot->SetAutoSize(false);
		SkillBarRootSlot->SetSize(LobbySkillBarLimit * (bMobileLike ? BaseScale : 1.0f));
	}

	if (SkillBarBackdropImage)
	{
		SkillBarBackdropImage->SetDesiredSizeOverride(LobbySkillBarLimit * (bMobileLike ? BaseScale : 1.0f));
	}

	if (MinimapRootSlot)
	{
		MinimapRootSlot->SetPosition(FVector2D(-18.0f, 18.0f));
		MinimapRootSlot->SetSize(MinimapSize * (bMobileLike ? BaseScale : 1.0f));
	}

	for (int32 Index = 0; Index < SkillSlotBorders.Num(); ++Index)
	{
		if (UBorder* Border = SkillSlotBorders[Index])
		{
			Border->SetDesiredSizeScale(FVector2D(1.0f, 1.0f));
			Border->SetRenderScale(FVector2D(1.0f, 1.0f));
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
			CooldownText->SetText(FText::GetEmpty());
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
