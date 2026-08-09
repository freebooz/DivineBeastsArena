// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Frontend/Auth/UDBALoginFlowWidgetBase.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/AudioComponent.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Frontend/Settings/DBAFrontendSettings.h"
#include "GameDBA/UI/Controllers/DBAGameUIManager.h"
#include "GameDBA/UI/DBAUIFontUtils.h"
#include "Engine/Texture2D.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	const FLinearColor GoldText(0.95f, 0.77f, 0.36f, 1.0f);
	const FLinearColor DeepJade(0.015f, 0.11f, 0.085f, 0.92f);
	const FLinearColor SoftJade(0.05f, 0.28f, 0.20f, 0.78f);

	template<typename AssetType>
	AssetType* LoadAssetIfCookedAvailable(const TCHAR* ObjectPath)
	{
		if (!ObjectPath)
		{
			return nullptr;
		}

		const FSoftObjectPath SoftPath(ObjectPath);
		const FString PackageName = SoftPath.GetLongPackageName();
		if (PackageName.IsEmpty() || !FPackageName::DoesPackageExist(PackageName))
		{
			return nullptr;
		}

		return Cast<AssetType>(SoftPath.TryLoad());
	}

	UTextBlock* MakeLoginButtonLabel(UWidgetTree* WidgetTree, const FText& Label)
	{
		UTextBlock* TextBlock = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()) : nullptr;
		if (TextBlock)
		{
			TextBlock->SetText(Label);
			TextBlock->SetJustification(ETextJustify::Center);
			TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.18f, 0.08f, 0.0f, 1.0f)));

			FSlateFontInfo FontInfo = TextBlock->GetFont();
			FontInfo.Size = 42;
			FontInfo.OutlineSettings.OutlineSize = 1;
			FontInfo.OutlineSettings.OutlineColor = FLinearColor(1.0f, 0.86f, 0.42f, 0.65f);
			TextBlock->SetFont(FontInfo);
		}
		return TextBlock;
	}

	UTextBlock* MakeText(UWidgetTree* WidgetTree, const FName Name, const FText& Text, float Size, const FLinearColor& Color, ETextJustify::Type Justification = ETextJustify::Center)
	{
		UTextBlock* TextBlock = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name) : nullptr;
		if (TextBlock)
		{
			TextBlock->SetText(Text);
			TextBlock->SetJustification(Justification);
			TextBlock->SetColorAndOpacity(FSlateColor(Color));

			FSlateFontInfo FontInfo = TextBlock->GetFont();
			FontInfo.Size = FMath::RoundToInt(Size);
			FontInfo.OutlineSettings.OutlineSize = Size >= 36.0f ? 2 : 0;
			FontInfo.OutlineSettings.OutlineColor = FLinearColor(0.05f, 0.025f, 0.0f, 0.82f);
			TextBlock->SetFont(FontInfo);
		}
		return TextBlock;
	}

	void AddCanvasChild(UCanvasPanel* Canvas, UWidget* Child, const FVector2D& Position, const FVector2D& Size, const FVector2D& Alignment = FVector2D::ZeroVector)
	{
		if (!Canvas || !Child)
		{
			return;
		}

		if (UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child))
		{
			Slot->SetAutoSize(false);
			Slot->SetPosition(Position);
			Slot->SetSize(Size);
			Slot->SetAlignment(Alignment);
		}
	}

	void AddCanvasChildAnchored(UCanvasPanel* Canvas, UWidget* Child, const FAnchors& Anchors, const FMargin& Offsets, const FVector2D& Alignment = FVector2D::ZeroVector)
	{
		if (!Canvas || !Child)
		{
			return;
		}

		if (UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child))
		{
			Slot->SetAnchors(Anchors);
			Slot->SetOffsets(Offsets);
			Slot->SetAlignment(Alignment);
		}
	}

	FSlateBrush MakeFullBleedTextureBrush(UTexture2D* Texture, const FVector2D& LayoutSize)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(Texture);
		if (LayoutSize.X > 1.0f && LayoutSize.Y > 1.0f)
		{
			Brush.ImageSize = LayoutSize;
		}
		else if (Texture)
		{
			Brush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
		}
		else
		{
			Brush.ImageSize = FVector2D(1920.0f, 1080.0f);
		}
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.Tiling = ESlateBrushTileType::NoTile;
		Brush.Margin = FMargin(0.0f);
		Brush.TintColor = FSlateColor(FLinearColor::White);
		return Brush;
	}

	void StretchWidgetToParentEdges(UWidget* Widget, const int32 CanvasZOrder = -2000)
	{
		if (!Widget)
		{
			return;
		}

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
		{
			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			CanvasSlot->SetOffsets(FMargin(0.0f));
			CanvasSlot->SetAlignment(FVector2D::ZeroVector);
			CanvasSlot->SetZOrder(CanvasZOrder);
			return;
		}

		if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Widget->Slot))
		{
			OverlaySlot->SetPadding(FMargin(0.0f));
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
			OverlaySlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	void ApplyFullBleedTextureToImage(UImage* Image, UTexture2D* Texture, const FVector2D& LayoutSize = FVector2D::ZeroVector)
	{
		if (!Image || !Texture)
		{
			return;
		}

		Image->SetBrush(MakeFullBleedTextureBrush(Texture, LayoutSize));
		Image->SetDesiredSizeOverride(FVector2D::ZeroVector);
		Image->SetColorAndOpacity(FLinearColor::White);
		Image->SetVisibility(ESlateVisibility::Visible);
		StretchWidgetToParentEdges(Image, -3000);
	}

	void ApplyFullBleedTextureToBorder(UBorder* Border, UTexture2D* Texture, const FVector2D& LayoutSize = FVector2D::ZeroVector)
	{
		if (!Border || !Texture)
		{
			return;
		}

		Border->SetBrush(MakeFullBleedTextureBrush(Texture, LayoutSize));
		Border->SetBrushColor(FLinearColor::White);
		Border->SetPadding(FMargin(0.0f));
		Border->SetVisibility(ESlateVisibility::Visible);
		StretchWidgetToParentEdges(Border, -3000);
	}

	void ApplyDesignScaledPointCanvasSlot(
		UWidget* Widget,
		const float AnchorX,
		const float AnchorY,
		const float DesignWidth,
		const float DesignHeight,
		const float LayoutWidth,
		const float LayoutHeight,
		const FVector2D& Alignment = FVector2D(0.5f, 0.5f),
		const int32 ZOrder = 0)
	{
		if (!Widget || LayoutWidth <= 1.0f || LayoutHeight <= 1.0f)
		{
			return;
		}

		const float ScaleX = LayoutWidth / 1920.0f;
		const float ScaleY = LayoutHeight / 1080.0f;
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Widget->Slot))
		{
			Slot->SetAutoSize(false);
			Slot->SetAnchors(FAnchors(AnchorX, AnchorY, AnchorX, AnchorY));
			Slot->SetAlignment(Alignment);
			Slot->SetPosition(FVector2D::ZeroVector);
			Slot->SetSize(FVector2D(DesignWidth * ScaleX, DesignHeight * ScaleY));
			Slot->SetOffsets(FMargin(0.0f));
			Slot->SetZOrder(ZOrder);
		}
	}

	void ApplyDesignScaledStretchCanvasSlot(
		UWidget* Widget,
		const FAnchors& Anchors,
		const FMargin& DesignOffsets,
		const float LayoutWidth,
		const float LayoutHeight,
		const int32 ZOrder = 0)
	{
		if (!Widget || LayoutWidth <= 1.0f || LayoutHeight <= 1.0f)
		{
			return;
		}

		const float ScaleX = LayoutWidth / 1920.0f;
		const float ScaleY = LayoutHeight / 1080.0f;
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Widget->Slot))
		{
			Slot->SetAutoSize(false);
			Slot->SetAnchors(Anchors);
			Slot->SetAlignment(FVector2D::ZeroVector);
			Slot->SetOffsets(FMargin(
				DesignOffsets.Left * ScaleX,
				DesignOffsets.Top * ScaleY,
				DesignOffsets.Right * ScaleX,
				DesignOffsets.Bottom * ScaleY));
			Slot->SetZOrder(ZOrder);
		}
	}

	UWidget* FindWidgetByNames(UWidgetTree* WidgetTree, const TArray<FName>& Names)
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

	bool StringContainsAny(const FString& Source, const TArray<FString>& Keywords)
	{
		for (const FString& Keyword : Keywords)
		{
			if (!Keyword.IsEmpty() && Source.Contains(Keyword))
			{
				return true;
			}
		}
		return false;
	}

	FString ExtractTextFromWidget(UWidget* Widget)
	{
		if (!Widget)
		{
			return FString();
		}

		if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			return TextBlock->GetText().ToString();
		}

		if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
		{
			const int32 Count = Panel->GetChildrenCount();
			for (int32 Index = 0; Index < Count; ++Index)
			{
				if (UWidget* Child = Panel->GetChildAt(Index))
				{
					const FString ChildText = ExtractTextFromWidget(Child);
					if (!ChildText.IsEmpty())
					{
						return ChildText;
					}
				}
			}
		}

		return FString();
	}

	UButton* FindButtonByHeuristics(
		UWidgetTree* WidgetTree,
		const TArray<FString>& NameKeywords,
		const TArray<FString>& LabelKeywords)
	{
		if (!WidgetTree)
		{
			return nullptr;
		}

		TArray<UWidget*> AllWidgets;
		WidgetTree->GetAllWidgets(AllWidgets);
		for (UWidget* Widget : AllWidgets)
		{
			UButton* Button = Cast<UButton>(Widget);
			if (!Button)
			{
				continue;
			}

			const FString ButtonName = Button->GetName();
			if (StringContainsAny(ButtonName, NameKeywords))
			{
				return Button;
			}

			if (UWidget* Content = Button->GetContent())
			{
				const FString LabelText = ExtractTextFromWidget(Content);
				if (StringContainsAny(LabelText, LabelKeywords))
				{
					return Button;
				}
			}
		}

		return nullptr;
	}

	void AddVerticalChild(UVerticalBox* Box, UWidget* Child, const FMargin& Padding, EHorizontalAlignment HorizontalAlignment = HAlign_Fill)
	{
		if (!Box || !Child)
		{
			return;
		}

		if (UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Child))
		{
			Slot->SetPadding(Padding);
			Slot->SetHorizontalAlignment(HorizontalAlignment);
		}
	}

	UButton* MakeTextButton(UWidgetTree* WidgetTree, const FName Name, const FText& Label, float FontSize, const FLinearColor& TextColor)
	{
		UButton* Button = WidgetTree ? WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name) : nullptr;
		if (!Button)
		{
			return nullptr;
		}

		FButtonStyle Style = Button->GetStyle();
		FSlateBrush NormalBrush;
		NormalBrush.DrawAs = ESlateBrushDrawType::Box;
		NormalBrush.TintColor = FSlateColor(SoftJade);
		Style.SetNormal(NormalBrush);

		FSlateBrush HoveredBrush = NormalBrush;
		HoveredBrush.TintColor = FSlateColor(FLinearColor(0.09f, 0.42f, 0.31f, 0.88f));
		Style.SetHovered(HoveredBrush);

		FSlateBrush PressedBrush = NormalBrush;
		PressedBrush.TintColor = FSlateColor(FLinearColor(0.02f, 0.16f, 0.12f, 0.96f));
		Style.SetPressed(PressedBrush);
		Button->SetStyle(Style);

		Button->AddChild(MakeText(WidgetTree, NAME_None, Label, FontSize, TextColor));
		return Button;
	}

	void ApplyEditableBoxStyle(UEditableTextBox* TextBox, const float MinDesiredWidth)
	{
		if (!TextBox)
		{
			return;
		}

		TextBox->SetMinDesiredWidth(MinDesiredWidth);
		TextBox->SetVisibility(ESlateVisibility::Visible);
	}

	void ResizeCanvasChild(UWidget* Widget, const FVector2D& Position, const FVector2D& Size)
	{
		if (!Widget)
		{
			return;
		}

		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Widget->Slot))
		{
			Slot->SetAutoSize(false);
			Slot->SetPosition(Position);
			Slot->SetSize(Size);
		}
	}

	void ApplyCenteredFractionCanvasPanelSlot(
		UCanvasPanelSlot* PanelSlot,
		const float WidthFraction,
		const float HeightFraction)
	{
		if (!PanelSlot)
		{
			return;
		}

		const float ClampedWidthFraction = FMath::Clamp(WidthFraction, 0.55f, 0.98f);
		const float ClampedHeightFraction = FMath::Clamp(HeightFraction, 0.55f, 0.98f);
		const float HalfWidth = ClampedWidthFraction * 0.5f;
		const float HalfHeight = ClampedHeightFraction * 0.5f;

		PanelSlot->SetAutoSize(false);
		PanelSlot->SetAnchors(FAnchors(
			0.5f - HalfWidth,
			0.5f - HalfHeight,
			0.5f + HalfWidth,
			0.5f + HalfHeight));
		PanelSlot->SetOffsets(FMargin(0.0f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetPosition(FVector2D::ZeroVector);
		PanelSlot->SetSize(FVector2D::ZeroVector);
	}

	void ApplyCenteredPointCanvasPanelSlot(
		UCanvasPanelSlot* PanelSlot,
		const float Width,
		const float Height)
	{
		if (!PanelSlot || Width <= 1.0f || Height <= 1.0f)
		{
			return;
		}

		PanelSlot->SetAutoSize(false);
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetPosition(FVector2D::ZeroVector);
		PanelSlot->SetSize(FVector2D(Width, Height));
		PanelSlot->SetOffsets(FMargin(0.0f));
	}

	void ApplyCenteredCanvasPanelSlot(
		UCanvasPanelSlot* PanelSlot,
		const float Width,
		const float Height,
		const float ParentWidth,
		const float ParentHeight)
	{
		if (!PanelSlot || ParentWidth <= 0.0f || ParentHeight <= 0.0f || Width <= 0.0f || Height <= 0.0f)
		{
			return;
		}

		ApplyCenteredFractionCanvasPanelSlot(
			PanelSlot,
			Width / ParentWidth,
			Height / ParentHeight);
	}

	bool IsCompactLoginViewport(const FVector2D& ViewportSize, const FDBALoginVisualLayoutSpec& Spec)
	{
		return ViewportSize.X > 0.0f
			&& ViewportSize.Y > 0.0f
			&& (ViewportSize.X <= Spec.CompactViewportWidthThreshold || ViewportSize.Y <= Spec.CompactViewportHeightThreshold);
	}

	void ApplyCompactLoginFormLayout(
		UWidgetTree* WidgetTree,
		const FDBALoginVisualLayoutSpec& Spec,
		const float PanelWidth,
		const float PanelHeight,
		const float InputRowHeight,
		const float InputEditableHeight,
		const float PanelPaddingValue,
		UEditableTextBox* EmailInput,
		UEditableTextBox* PasswordInput,
		UWidget* PasswordVisibilityButton,
		UButton* LoginButton,
		UButton* GuestLoginButton,
		UButton* RegisterAccountButton)
	{
		if (!WidgetTree)
		{
			return;
		}

		const float ScaleX = PanelWidth / Spec.PanelWidth;
		const float ScaleY = PanelHeight / Spec.PanelHeight;
		const auto Pos = [ScaleX, ScaleY](const float X, const float Y)
		{
			return FVector2D(X * ScaleX, Y * ScaleY);
		};
		const auto Size = [ScaleX, ScaleY](const float W, const float H)
		{
			return FVector2D(W * ScaleX, H * ScaleY);
		};

		const float InputBgWidth = PanelWidth - PanelPaddingValue * 2.0f - 24.0f * ScaleX;
		const float AccountInputWidth = FMath::Max(180.0f * ScaleX, InputBgWidth - 180.0f * ScaleX);
		const float PasswordInputWidth = FMath::Max(160.0f * ScaleX, InputBgWidth - 220.0f * ScaleX);

		if (UWidget* ServerLabel = FindWidgetByNames(WidgetTree, { TEXT("ReferenceServerLabel") }))
		{
			ResizeCanvasChild(ServerLabel, Pos(10.0f, 3.0f), Size(118.0f, 48.0f));
		}
		if (UWidget* ServerSelectButton = FindWidgetByNames(WidgetTree, { TEXT("ServerSelectButton") }))
		{
			ResizeCanvasChild(ServerSelectButton, Pos(160.0f, 0.0f), Size(322.0f, 56.0f));
		}
		if (UWidget* AccountBg = FindWidgetByNames(WidgetTree, { TEXT("ReferenceAccountInputBg") }))
		{
			ResizeCanvasChild(AccountBg, Pos(20.0f, 80.0f), FVector2D(InputBgWidth, InputRowHeight));
		}
		if (UWidget* AccountLabel = FindWidgetByNames(WidgetTree, { TEXT("ReferenceAccountLabel") }))
		{
			ResizeCanvasChild(AccountLabel, Pos(36.0f, 94.0f), Size(132.0f, 36.0f));
		}
		if (EmailInput)
		{
			ResizeCanvasChild(EmailInput, Pos(200.0f, 86.0f), FVector2D(AccountInputWidth, InputEditableHeight));
		}
		if (UWidget* PasswordBg = FindWidgetByNames(WidgetTree, { TEXT("ReferencePasswordInputBg") }))
		{
			ResizeCanvasChild(PasswordBg, Pos(20.0f, 166.0f), FVector2D(InputBgWidth, InputRowHeight));
		}
		if (UWidget* PasswordLabel = FindWidgetByNames(WidgetTree, { TEXT("ReferencePasswordLabel") }))
		{
			ResizeCanvasChild(PasswordLabel, Pos(36.0f, 180.0f), Size(132.0f, 36.0f));
		}
		if (PasswordInput)
		{
			ResizeCanvasChild(PasswordInput, Pos(200.0f, 172.0f), FVector2D(PasswordInputWidth, InputEditableHeight));
		}
		if (PasswordVisibilityButton)
		{
			const float PasswordButtonX = 200.0f * ScaleX + PasswordInputWidth + 12.0f;
			const float PasswordButtonY = 176.0f * ScaleY;
			ResizeCanvasChild(
				PasswordVisibilityButton,
				FVector2D(PasswordButtonX, PasswordButtonY),
				FVector2D(64.0f * ScaleX, InputEditableHeight));
		}
		if (UWidget* RememberToggle = FindWidgetByNames(WidgetTree, { TEXT("RememberToggleButton") }))
		{
			ResizeCanvasChild(RememberToggle, Pos(20.0f, 246.0f), Size(42.0f, 42.0f));
		}
		if (UWidget* RememberLabel = FindWidgetByNames(WidgetTree, { TEXT("ReferenceRememberText") }))
		{
			ResizeCanvasChild(RememberLabel, Pos(72.0f, 250.0f), Size(120.0f, 34.0f));
		}
		if (UWidget* ForgotPassword = FindWidgetByNames(WidgetTree, { TEXT("ForgotPasswordButton") }))
		{
			ResizeCanvasChild(ForgotPassword, Pos(508.0f, 244.0f), Size(128.0f, 42.0f));
		}
		if (LoginButton)
		{
			ResizeCanvasChild(LoginButton, Pos(96.0f, 296.0f), Size(472.0f, 86.0f));
		}
		if (GuestLoginButton)
		{
			ResizeCanvasChild(GuestLoginButton, Pos(174.0f, 394.0f), Size(316.0f, 66.0f));
		}
		if (RegisterAccountButton)
		{
			ResizeCanvasChild(RegisterAccountButton, Pos(226.0f, 468.0f), Size(212.0f, 46.0f));
		}
	}

	void ApplyTopCenteredCanvasPanelSlot(
		UCanvasPanelSlot* PanelSlot,
		const float AnchorX,
		const float TopOffset,
		const float Width,
		const float Height)
	{
		if (!PanelSlot)
		{
			return;
		}

		PanelSlot->SetAutoSize(false);
		PanelSlot->SetAnchors(FAnchors(AnchorX, 0.0f, AnchorX, 0.0f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		PanelSlot->SetPosition(FVector2D(0.0f, TopOffset));
		PanelSlot->SetSize(FVector2D(Width, Height));
	}

	FVector2D GetLoginWidgetViewportSize(const UUserWidget* Widget)
	{
		if (!Widget)
		{
			return FVector2D::ZeroVector;
		}

		// 始终优先读取真实游戏视口，避免用控件当前几何尺寸形成“缩在左上角”的反馈环。
		if (const UWorld* World = Widget->GetWorld())
		{
			if (const UGameViewportClient* ViewportClient = World->GetGameViewport())
			{
				FVector2D ViewportSize = FVector2D::ZeroVector;
				ViewportClient->GetViewportSize(ViewportSize);
				if (ViewportSize.X > 1.0f && ViewportSize.Y > 1.0f)
				{
					return ViewportSize;
				}
			}
		}

		if (GEngine && GEngine->GameViewport)
		{
			FVector2D EngineViewportSize = FVector2D::ZeroVector;
			GEngine->GameViewport->GetViewportSize(EngineViewportSize);
			if (EngineViewportSize.X > 1.0f && EngineViewportSize.Y > 1.0f)
			{
				return EngineViewportSize;
			}
		}

		const FVector2D LayoutViewportSize = UWidgetLayoutLibrary::GetViewportSize(const_cast<UUserWidget*>(Widget));
		if (LayoutViewportSize.X > 1.0f && LayoutViewportSize.Y > 1.0f)
		{
			return LayoutViewportSize;
		}

		return FVector2D::ZeroVector;
	}

	bool LoginWidgetGeometryMatchesViewport(const UUserWidget* Widget, const FVector2D& GameViewportSize)
	{
		if (!Widget || !Widget->IsInViewport() || GameViewportSize.X <= 1.0f || GameViewportSize.Y <= 1.0f)
		{
			return false;
		}

		const FVector2D LocalSize = Widget->GetCachedGeometry().GetLocalSize();
		if (LocalSize.X <= 16.0f || LocalSize.Y <= 16.0f)
		{
			return false;
		}

		const float ViewportScale = FMath::Max(
			UWidgetLayoutLibrary::GetViewportScale(const_cast<UUserWidget*>(Widget)),
			KINDA_SMALL_NUMBER);
		const FVector2D ExpectedLogicalSize = GameViewportSize / ViewportScale;
		// DPI 缩放下几何尺寸与视口像素不必相等，只要同比例铺满即可。
		return FMath::IsNearlyEqual(LocalSize.X, ExpectedLogicalSize.X, 4.0f)
			&& FMath::IsNearlyEqual(LocalSize.Y, ExpectedLogicalSize.Y, 4.0f);
	}

	FVector2D GetLoginLayoutCanvasSize(const UUserWidget* Widget, const FVector2D& FallbackViewportSize)
	{
		if (LoginWidgetGeometryMatchesViewport(Widget, FallbackViewportSize))
		{
			return Widget->GetCachedGeometry().GetLocalSize();
		}

		return FallbackViewportSize;
	}

	void EnsureLoginWidgetFillsGameViewport(UUserWidget* Widget)
	{
		if (!Widget)
		{
			return;
		}

		// UE5 SetDesiredSizeInViewport 会把锚点重置为 (0,0) 点锚点，因此拉伸锚点必须最后设置。
		Widget->SetAlignmentInViewport(FVector2D::ZeroVector);
		Widget->SetPositionInViewport(FVector2D::ZeroVector, false);
		Widget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		Widget->SetRenderScale(FVector2D(1.0f, 1.0f));
		Widget->SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	}

	UCanvasPanelSlot* ResolveLoginPanelCanvasSlot(UWidget* PanelWidget)
	{
		if (!PanelWidget)
		{
			return nullptr;
		}

		return Cast<UCanvasPanelSlot>(PanelWidget->Slot);
	}

	void StretchLoginBackgroundWidgets(UWidgetTree* WidgetTree, const FVector2D& LayoutSize, UTexture2D* BackgroundTexture = nullptr)
	{
		if (!WidgetTree)
		{
			return;
		}

		const TArray<FName> BackgroundNames = {
			TEXT("ForestBackgroundImage"),
			TEXT("LoginBackgroundImage"),
			TEXT("BackgroundImage"),
			TEXT("ReferenceForestBackground")
		};

		for (const FName BackgroundName : BackgroundNames)
		{
			if (UImage* BackgroundImage = Cast<UImage>(WidgetTree->FindWidget(BackgroundName)))
			{
				if (BackgroundTexture)
				{
					ApplyFullBleedTextureToImage(BackgroundImage, BackgroundTexture, LayoutSize);
				}
				else
				{
					StretchWidgetToParentEdges(BackgroundImage, -3000);
				}
			}
		}

		if (UWidget* VignetteWidget = WidgetTree->FindWidget(TEXT("DarkVignetteOverlay")))
		{
			StretchWidgetToParentEdges(VignetteWidget, -2500);
		}
	}

	void ApplyLoginFrameBorderLayout(UWidgetTree* WidgetTree, const float LayoutWidth, const float LayoutHeight)
	{
		if (!WidgetTree || LayoutWidth <= 1.0f || LayoutHeight <= 1.0f)
		{
			return;
		}

		ApplyDesignScaledStretchCanvasSlot(
			WidgetTree->FindWidget(TEXT("FrameTopLine")),
			FAnchors(0.0f, 0.0f, 1.0f, 0.0f),
			FMargin(6.0f, 6.0f, -12.0f, 2.0f),
			LayoutWidth,
			LayoutHeight,
			-1200);
		ApplyDesignScaledStretchCanvasSlot(
			WidgetTree->FindWidget(TEXT("FrameBottomLine")),
			FAnchors(0.0f, 1.0f, 1.0f, 1.0f),
			FMargin(6.0f, -8.0f, -12.0f, 2.0f),
			LayoutWidth,
			LayoutHeight,
			-1200);
		ApplyDesignScaledStretchCanvasSlot(
			WidgetTree->FindWidget(TEXT("FrameLeftLine")),
			FAnchors(0.0f, 0.0f, 0.0f, 1.0f),
			FMargin(6.0f, 6.0f, 2.0f, -12.0f),
			LayoutWidth,
			LayoutHeight,
			-1200);
		ApplyDesignScaledStretchCanvasSlot(
			WidgetTree->FindWidget(TEXT("FrameRightLine")),
			FAnchors(1.0f, 0.0f, 1.0f, 1.0f),
			FMargin(-8.0f, 6.0f, 2.0f, -12.0f),
			LayoutWidth,
			LayoutHeight,
			-1200);
	}

	void ApplyLoginChromeWidgetsLayout(UWidgetTree* WidgetTree, const float LayoutWidth, const float LayoutHeight, const bool bVeryCompactViewport)
	{
		if (!WidgetTree || LayoutWidth <= 1.0f || LayoutHeight <= 1.0f || bVeryCompactViewport)
		{
			return;
		}

		const float ScaleX = LayoutWidth / 1920.0f;
		const float ScaleY = LayoutHeight / 1080.0f;

		ApplyDesignScaledPointCanvasSlot(
			WidgetTree->FindWidget(TEXT("TitleBackplate")),
			0.5f,
			0.035f,
			820.0f,
			160.0f,
			LayoutWidth,
			LayoutHeight,
			FVector2D(0.5f, 0.0f),
			100);
		ApplyDesignScaledPointCanvasSlot(
			WidgetTree->FindWidget(TEXT("TitleText")),
			0.5f,
			0.06f,
			760.0f,
			120.0f,
			LayoutWidth,
			LayoutHeight,
			FVector2D(0.5f, 0.0f),
			110);

		if (UWidget* RightToolPanel = WidgetTree->FindWidget(TEXT("RightToolPanel")))
		{
			RightToolPanel->SetVisibility(ESlateVisibility::Visible);
			ApplyDesignScaledPointCanvasSlot(
				RightToolPanel,
				0.94f,
				0.42f,
				118.0f,
				360.0f,
				LayoutWidth,
				LayoutHeight,
				FVector2D(0.5f, 0.0f),
				200);

			const float ToolButtonWidth = 118.0f * ScaleX;
			const float ToolButtonHeight = 96.0f * ScaleY;
			const float ToolButtonSpacing = 120.0f * ScaleY;
			const TArray<FName> ToolButtonNames = {
				TEXT("NoticeToolButton"),
				TEXT("SupportToolButton"),
				TEXT("RepairToolButton")
			};
			for (int32 ToolIndex = 0; ToolIndex < ToolButtonNames.Num(); ++ToolIndex)
			{
				if (UWidget* ToolButton = WidgetTree->FindWidget(ToolButtonNames[ToolIndex]))
				{
					ToolButton->SetVisibility(ESlateVisibility::Visible);
					ResizeCanvasChild(
						ToolButton,
						FVector2D(0.0f, ToolIndex * ToolButtonSpacing),
						FVector2D(ToolButtonWidth, ToolButtonHeight));
				}
			}
		}

		ApplyDesignScaledPointCanvasSlot(
			WidgetTree->FindWidget(TEXT("AgeRatingPanel")),
			0.03f,
			0.82f,
			100.0f,
			132.0f,
			LayoutWidth,
			LayoutHeight,
			FVector2D(0.0f, 0.0f),
			150);
		ApplyDesignScaledPointCanvasSlot(
			WidgetTree->FindWidget(TEXT("AgreementText")),
			0.5f,
			0.91f,
			760.0f,
			42.0f,
			LayoutWidth,
			LayoutHeight,
			FVector2D(0.5f, 0.0f),
			160);
	}

	UCanvasPanel* FindLoginRootCanvas(UWidgetTree* WidgetTree, UWidget* LoginPanelWidget)
	{
		if (LoginPanelWidget)
		{
			UWidget* Current = LoginPanelWidget;
			UCanvasPanel* RootCanvas = nullptr;
			while (Current)
			{
				if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(Current))
				{
					RootCanvas = Canvas;
				}
				Current = Current->GetParent();
			}
			return RootCanvas;
		}

		if (!WidgetTree)
		{
			return nullptr;
		}

		const TArray<FName> RootCanvasNames = {
			TEXT("ReferenceLoginRoot"),
			TEXT("RootCanvas_Auto"),
			TEXT("RootCanvas")
		};
		for (const FName RootCanvasName : RootCanvasNames)
		{
			if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(WidgetTree->FindWidget(RootCanvasName)))
			{
				return Canvas;
			}
		}

		return Cast<UCanvasPanel>(WidgetTree->RootWidget);
	}

	void SyncLoginRootCanvasToViewport(UWidgetTree* WidgetTree, UWidget* LoginPanelWidget, const FVector2D& ViewportSize, const bool bUseNativeLayout)
	{
		if (!WidgetTree || ViewportSize.X <= 1.0f || ViewportSize.Y <= 1.0f)
		{
			return;
		}

		if (bUseNativeLayout)
		{
			if (USizeBox* DesignRoot = Cast<USizeBox>(WidgetTree->RootWidget))
			{
				DesignRoot->SetWidthOverride(ViewportSize.X);
				DesignRoot->SetHeightOverride(ViewportSize.Y);
			}
			return;
		}

		if (USizeBox* SizeRoot = Cast<USizeBox>(WidgetTree->RootWidget))
		{
			SizeRoot->SetWidthOverride(ViewportSize.X);
			SizeRoot->SetHeightOverride(ViewportSize.Y);
		}

		if (UCanvasPanel* RootCanvas = FindLoginRootCanvas(WidgetTree, LoginPanelWidget))
		{
			if (UCanvasPanelSlot* RootSlot = Cast<UCanvasPanelSlot>(RootCanvas->Slot))
			{
				RootSlot->SetAutoSize(false);
				RootSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
				RootSlot->SetOffsets(FMargin(0.0f));
				RootSlot->SetAlignment(FVector2D::ZeroVector);
			}
		}
	}

	UWidget* FindLoginPanelWidget(UWidgetTree* WidgetTree)
	{
		if (!WidgetTree)
		{
			return nullptr;
		}

		const TArray<FName> CandidateNames = {
			TEXT("LoginPanel"),
			TEXT("LoginFormPanel"),
			TEXT("AuthPanel"),
			TEXT("Panel_Login"),
			TEXT("LoginCard")
		};

		for (const FName CandidateName : CandidateNames)
		{
			if (UWidget* Candidate = WidgetTree->FindWidget(CandidateName))
			{
				return Candidate;
			}
		}

		return nullptr;
	}

}

UDBALoginFlowWidgetBase::UDBALoginFlowWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseReferenceNativeLayout = false;
	AvailableServers = {
		FText::FromString(TEXT("苍穹之森")),
		FText::FromString(TEXT("星辉荒原")),
		FText::FromString(TEXT("青木幻林"))
	};
}

FDBALoginVisualLayoutSpec UDBALoginFlowWidgetBase::GetReferenceVisualLayoutSpec()
{
	FDBALoginVisualLayoutSpec Spec;
	Spec.PanelAnchorX = 0.50f;
	Spec.TitleText = FText::FromString(TEXT("\u795E\u517D\u7ADE\u6280\u573A"));
	Spec.PrimaryButtonText = FText::FromString(TEXT("\u767B\u5F55"));
	Spec.LeftToolLabels = {
		FText::FromString(TEXT("\u516C\u544A")),
		FText::FromString(TEXT("\u5BA2\u670D")),
		FText::FromString(TEXT("\u4FEE\u590D"))
	};
	return Spec;
}

float UDBALoginFlowWidgetBase::GetLoginFlowViewportScale() const
{
	const FDBALoginVisualLayoutSpec Spec = GetReferenceVisualLayoutSpec();
	const FVector2D ViewportSize = GetLoginWidgetViewportSize(this);
	if (ViewportSize.X <= 1.0f || ViewportSize.Y <= 1.0f)
	{
		return 1.0f;
	}

	const float FitScale = FMath::Min(
		ViewportSize.X / Spec.ReferenceDesignWidth,
		ViewportSize.Y / Spec.ReferenceDesignHeight);
	return FitScale;
}

void UDBALoginFlowWidgetBase::ApplyLoginViewportPresentation()
{
	const FVector2D ViewportSize = GetLoginWidgetViewportSize(this);
	EnsureLoginWidgetFillsGameViewport(this);
	ApplyLoginResponsiveLayout();

	const FVector2D LocalSize = GetCachedGeometry().GetLocalSize();
	const bool bGeometryMatches = LoginWidgetGeometryMatchesViewport(this, ViewportSize);
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] 视口铺满：游戏视口=%.0fx%.0f，控件几何=%.0fx%.0f，匹配=%s"),
		ViewportSize.X,
		ViewportSize.Y,
		LocalSize.X,
		LocalSize.Y,
		bGeometryMatches ? TEXT("是") : TEXT("否"));

}

FReply UDBALoginFlowWidgetBase::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FVector2D ScreenPosition = InMouseEvent.GetScreenSpacePosition();
	if (EmailInput && EmailInput->GetCachedGeometry().IsUnderLocation(ScreenPosition))
	{
		EmailInput->SetKeyboardFocus();
	}
	else if (PasswordInput && PasswordInput->GetCachedGeometry().IsUnderLocation(ScreenPosition))
	{
		PasswordInput->SetKeyboardFocus();
	}

	// 预览阶段只协助设置焦点，不消费事件；输入框仍需收到鼠标按下以进入编辑状态。
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

void UDBALoginFlowWidgetBase::ScheduleLoginLayoutRefresh()
{
	if (UWorld* World = GetWorld())
	{
		LoginLayoutRefreshRetryCount = 0;
		World->GetTimerManager().ClearTimer(DelayedLayoutRefreshTimerHandle);
		World->GetTimerManager().SetTimer(
			DelayedLayoutRefreshTimerHandle,
			this,
			&UDBALoginFlowWidgetBase::HandleDeferredLoginLayoutRefresh,
			0.05f,
			false);
	}
}

void UDBALoginFlowWidgetBase::HandleDeferredLoginLayoutRefresh()
{
	if (!IsInViewport())
	{
		++LoginLayoutRefreshRetryCount;
		if (LoginLayoutRefreshRetryCount < 8)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					DelayedLayoutRefreshTimerHandle,
					this,
					&UDBALoginFlowWidgetBase::HandleDeferredLoginLayoutRefresh,
					0.05f,
					false);
			}
		}
		return;
	}

	EnsureLoginWidgetFillsGameViewport(this);
	ApplyLoginResponsiveLayout();

	const FVector2D ViewportSize = GetLoginWidgetViewportSize(this);
	++LoginLayoutRefreshRetryCount;
	const bool bViewportReady = ViewportSize.X > 1.0f && ViewportSize.Y > 1.0f;
	const bool bGeometryMatchesViewport = LoginWidgetGeometryMatchesViewport(this, ViewportSize);
	const bool bNeedsGeometrySettleRetry = !bGeometryMatchesViewport && LoginLayoutRefreshRetryCount < 8;
	const bool bNeedsViewportRetry = !bViewportReady && LoginLayoutRefreshRetryCount < 8;
	if (bNeedsGeometrySettleRetry || bNeedsViewportRetry)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				DelayedLayoutRefreshTimerHandle,
				this,
				&UDBALoginFlowWidgetBase::HandleDeferredLoginLayoutRefresh,
				0.05f,
				false);
		}
		return;
	}

	FocusDefaultInput();
}

UEditableTextBox* UDBALoginFlowWidgetBase::GetDefaultInputFocusWidget() const
{
	return EmailInput;
}

void UDBALoginFlowWidgetBase::FocusDefaultInput()
{
	if (!EmailInput)
	{
		return;
	}

	EmailInput->SetIsEnabled(true);
	if (PasswordInput)
	{
		PasswordInput->SetIsEnabled(true);
	}
	if (APlayerController* OwningPlayer = GetOwningPlayer())
	{
		EmailInput->SetUserFocus(OwningPlayer);
	}
	EmailInput->SetKeyboardFocus();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DelayedFocusTimerHandle);
		World->GetTimerManager().SetTimer(
			DelayedFocusTimerHandle,
			[this]()
			{
				if (EmailInput && IsInViewport())
				{
					if (APlayerController* OwningPlayer = GetOwningPlayer())
					{
						EmailInput->SetUserFocus(OwningPlayer);
					}
					EmailInput->SetKeyboardFocus();
				}
			},
			0.1f,
			false);
	}
}

void UDBALoginFlowWidgetBase::EnsureReferenceDesignRootSizeBox()
{
	if (!WidgetTree || !bUseReferenceNativeLayout)
	{
		return;
	}

	const FDBALoginVisualLayoutSpec Spec = GetReferenceVisualLayoutSpec();
	if (USizeBox* ExistingRoot = Cast<USizeBox>(WidgetTree->RootWidget))
	{
		ExistingRoot->SetWidthOverride(Spec.ReferenceDesignWidth);
		ExistingRoot->SetHeightOverride(Spec.ReferenceDesignHeight);
		return;
	}

	if (!WidgetTree->RootWidget)
	{
		return;
	}

	USizeBox* DesignRoot = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("LoginReferenceDesignRoot"));
	DesignRoot->SetWidthOverride(Spec.ReferenceDesignWidth);
	DesignRoot->SetHeightOverride(Spec.ReferenceDesignHeight);
	DesignRoot->SetContent(WidgetTree->RootWidget);
	WidgetTree->RootWidget = DesignRoot;
}

void UDBALoginFlowWidgetBase::ApplyLoginResponsiveLayout()
{
	if (bUseComposedImageLayout)
	{
		return;
	}

	const FDBALoginVisualLayoutSpec Spec = GetReferenceVisualLayoutSpec();
	const FVector2D ViewportSize = GetLoginWidgetViewportSize(this);
	const bool bHasValidViewport = ViewportSize.X > 1.0f && ViewportSize.Y > 1.0f;
	const FVector2D LayoutCanvasSize = GetLoginLayoutCanvasSize(this, ViewportSize);
	const bool bHasValidLayoutCanvas = LayoutCanvasSize.X > 16.0f && LayoutCanvasSize.Y > 16.0f;
	const bool bCompactViewport = bHasValidViewport && IsCompactLoginViewport(ViewportSize, Spec);
	const bool bVeryCompactViewport = bHasValidViewport
		&& (ViewportSize.X <= 960.0f || ViewportSize.Y <= 540.0f);

	if (UWidget* ResolvedLoginPanel = FindLoginPanelWidget(WidgetTree))
	{
		LoginPanel = ResolvedLoginPanel;
	}

	const float LayoutWidth = bHasValidLayoutCanvas ? LayoutCanvasSize.X : (bHasValidViewport ? ViewportSize.X : Spec.ReferenceDesignWidth);
	const float LayoutHeight = bHasValidLayoutCanvas ? LayoutCanvasSize.Y : (bHasValidViewport ? ViewportSize.Y : Spec.ReferenceDesignHeight);

	SyncLoginRootCanvasToViewport(WidgetTree, LoginPanel, LayoutCanvasSize, bUseReferenceNativeLayout);
	StretchLoginBackgroundWidgets(WidgetTree, FVector2D(LayoutWidth, LayoutHeight), LoginBackgroundTexture);
	ApplyLoginFrameBorderLayout(WidgetTree, LayoutWidth, LayoutHeight);
	ApplyLoginChromeWidgetsLayout(WidgetTree, LayoutWidth, LayoutHeight, bVeryCompactViewport);

	const float PanelWidthFraction = 0.92f;
	const float PanelHeightFraction = bVeryCompactViewport ? 0.90f : 0.82f;

	const float PanelWidth = LayoutWidth * PanelWidthFraction;
	const float PanelHeight = LayoutHeight * PanelHeightFraction;
	const float InputEditableHeight = FMath::Clamp(LayoutHeight * 0.082f, 42.0f, 58.0f);
	const float InputRowHeight = InputEditableHeight + 12.0f;
	const float InputMinWidth = FMath::Max(420.0f, PanelWidth - Spec.PanelPadding * 2.0f - 48.0f);
	const float PanelPaddingValue = bCompactViewport ? Spec.PanelPadding * 0.72f : Spec.PanelPadding;

	ApplyEditableBoxStyle(EmailInput, InputMinWidth);
	ApplyEditableBoxStyle(PasswordInput, InputMinWidth);

	if (LoginPanel)
	{
		if (UCanvasPanelSlot* PanelSlot = ResolveLoginPanelCanvasSlot(LoginPanel))
		{
			ApplyCenteredPointCanvasPanelSlot(PanelSlot, PanelWidth, PanelHeight);
			UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] 登录面板居中布局：画布=%.0fx%.0f，面板=%.0fx%.0f，蓝图原生布局=%s"),
				LayoutWidth,
				LayoutHeight,
				PanelWidth,
				PanelHeight,
				bUseReferenceNativeLayout ? TEXT("是") : TEXT("否"));
		}
		else
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] 登录面板 %s 未找到 CanvasPanelSlot，无法运行时居中。"), *LoginPanel->GetName());
		}
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] 未找到 LoginPanel 控件，跳过居中布局。"));
	}

	if (UBorder* PanelBorder = Cast<UBorder>(LoginPanel))
	{
		PanelBorder->SetPadding(FMargin(PanelPaddingValue, PanelPaddingValue, PanelPaddingValue, PanelPaddingValue * 0.72f));
	}

	if (bUseReferenceNativeLayout)
	{
		ApplyCompactLoginFormLayout(
			WidgetTree,
			Spec,
			PanelWidth,
			PanelHeight,
			InputRowHeight,
			InputEditableHeight,
			PanelPaddingValue,
			EmailInput,
			PasswordInput,
			PasswordVisibilityButton,
			LoginButton,
			GuestLoginButton,
			RegisterAccountButton);
	}

	const auto HideWidgetIfFound = [this](const TArray<FName>& Names)
	{
		if (UWidget* Widget = FindWidgetByNames(WidgetTree, Names))
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
	};

	if (bVeryCompactViewport)
	{
		HideWidgetIfFound({ TEXT("ReferenceSideTools"), TEXT("RightToolPanel") });
		HideWidgetIfFound({ TEXT("ReferenceLoginSubtitle") });
		HideWidgetIfFound({ TEXT("TitleText") });
		HideWidgetIfFound({ TEXT("NoticeToolButton"), TEXT("SupportToolButton"), TEXT("RepairToolButton") });
	}
	else if (TitleText)
	{
		TitleText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UDBALoginFlowWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBALoginFlowWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	const UDBAFrontendSettings* FrontendSettings = GetDefault<UDBAFrontendSettings>();
	bRememberAccount = !FrontendSettings || FrontendSettings->bRememberSessionByDefault;
	FViewport::ViewportResizedEvent.RemoveAll(this);
	FViewport::ViewportResizedEvent.AddUObject(this, &UDBALoginFlowWidgetBase::HandleViewportResized);

	InitializeVisualAssets();
	const bool bHasBlueprintLayout = WidgetTree && WidgetTree->RootWidget != nullptr;
	if (bHasBlueprintLayout && FindLoginPanelWidget(WidgetTree))
	{
		// WBP_DBA_Login 蓝图固定 LoginPanel 锚点约为 (0.5, 0.32)，运行时必须在 C++ 侧重算居中。
		bUseReferenceNativeLayout = false;
	}
	if (bUseReferenceNativeLayout)
	{
		BuildReferenceNativeLayout();
	}
	else if (bHasBlueprintLayout)
	{
		LoginPanel = FindLoginPanelWidget(WidgetTree);
		EmailInput = Cast<UEditableTextBox>(FindWidgetByNames(WidgetTree, { TEXT("EmailInput") }));
		PasswordInput = Cast<UEditableTextBox>(FindWidgetByNames(WidgetTree, { TEXT("PasswordInput") }));
		LoginButton = Cast<UButton>(FindWidgetByNames(WidgetTree, {
			TEXT("LoginButton"),
			TEXT("BtnLogin"),
			TEXT("LoginBtn"),
			TEXT("Button_Login"),
			TEXT("PrimaryLoginButton")
		}));
		GuestLoginButton = Cast<UButton>(FindWidgetByNames(WidgetTree, {
			TEXT("GuestLoginButton"),
			TEXT("GuestButton"),
			TEXT("BtnGuestLogin"),
			TEXT("Button_GuestLogin"),
			TEXT("VisitorLoginButton"),
			TEXT("TouristLoginButton")
		}));
		ErrorText = Cast<UTextBlock>(FindWidgetByNames(WidgetTree, { TEXT("ErrorText") }));
		StatusText = Cast<UTextBlock>(FindWidgetByNames(WidgetTree, { TEXT("StatusText") }));
		TitleText = Cast<UTextBlock>(FindWidgetByNames(WidgetTree, { TEXT("TitleText") }));

		if (!LoginButton)
		{
			LoginButton = FindButtonByHeuristics(
				WidgetTree,
				{ TEXT("Login"), TEXT("SignIn"), TEXT("Enter") },
				{ TEXT("Login"), TEXT("Sign In"), TEXT("\u767b\u5f55"), TEXT("\u8fdb\u5165") });
		}
		if (!GuestLoginButton)
		{
			GuestLoginButton = FindButtonByHeuristics(
				WidgetTree,
				{ TEXT("Guest"), TEXT("Visitor"), TEXT("Tourist"), TEXT("Passport") },
				{ TEXT("Guest"), TEXT("Visitor"), TEXT("Tourist"), TEXT("\u6e38\u5ba2"), TEXT("\u901a\u884c\u8bc1"), TEXT("\u795e\u517d") });
		}
		if (!EmailInput)
		{
			if (UCanvasPanel* EmailHost = Cast<UCanvasPanel>(FindWidgetByNames(WidgetTree, { TEXT("EmailInputHost") })))
			{
				EmailInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("EmailInput"));
				EmailInput->SetHintText(FText::FromString(TEXT("\u8BF7\u8F93\u5165\u8D26\u53F7")));
				ApplyEditableBoxStyle(EmailInput, GetReferenceVisualLayoutSpec().InputMinDesiredWidth);
				AddCanvasChildAnchored(EmailHost, EmailInput, FAnchors(0.0f, 0.0f, 1.0f, 1.0f), FMargin(0.0f, 0.0f, 0.0f, 0.0f));
			}
		}

		if (!PasswordInput)
		{
			if (UCanvasPanel* PasswordHost = Cast<UCanvasPanel>(FindWidgetByNames(WidgetTree, { TEXT("PasswordInputHost") })))
			{
				PasswordInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("PasswordInput"));
				PasswordInput->SetHintText(FText::FromString(TEXT("\u8BF7\u8F93\u5165\u5BC6\u7801")));
				PasswordInput->SetIsPassword(true);
				ApplyEditableBoxStyle(PasswordInput, GetReferenceVisualLayoutSpec().InputMinDesiredWidth);
				AddCanvasChildAnchored(PasswordHost, PasswordInput, FAnchors(0.0f, 0.0f, 1.0f, 1.0f), FMargin(0.0f, 0.0f, 0.0f, 0.0f));
			}
		}
	}
	else
	{
		EnsureNativeFallbackLayout();
	}
	BindButtonClickAudio();
	BindControls();
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] 组件绑定状态：登录按钮=%s，游客登录按钮=%s，账号输入框=%s，密码输入框=%s"),
		LoginButton ? *LoginButton->GetName() : TEXT("NULL"),
		GuestLoginButton ? *GuestLoginButton->GetName() : TEXT("NULL"),
		EmailInput ? *EmailInput->GetName() : TEXT("NULL"),
		PasswordInput ? *PasswordInput->GetName() : TEXT("NULL"));

	InitializeAudioAssets();
	ApplyVisualStyle();
	DBAUIFonts::ApplyGameFontToWidgetTree(WidgetTree);
	ClearError();

	if (UDBAFrontendFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowStateChanged);
		LoginFlow->OnFlowStateChanged.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowStateChanged);
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowError);
		LoginFlow->OnFlowError.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowError);
		HandleFlowStateChanged(LoginFlow->GetFlowState());
	}

	if (IsInViewport())
	{
		ApplyLoginViewportPresentation();
	}
	ScheduleLoginLayoutRefresh();
}

void UDBALoginFlowWidgetBase::NativeDestruct()
{
	FViewport::ViewportResizedEvent.RemoveAll(this);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DelayedFocusTimerHandle);
		World->GetTimerManager().ClearTimer(DelayedLayoutRefreshTimerHandle);
	}

	if (UDBAFrontendFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowStateChanged);
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowError);
	}

	UnbindControls();
	Super::NativeDestruct();
}

void UDBALoginFlowWidgetBase::HandleViewportResized(FViewport* Viewport, uint32 Unused)
{
	(void)Viewport;
	(void)Unused;
	if (!IsInViewport())
	{
		return;
	}

	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] 检测到视口尺寸变化，重新计算登录布局。"));
	ScheduleLoginLayoutRefresh();
}

void UDBALoginFlowWidgetBase::EnsureNativeFallbackLayout()
{
	if (!WidgetTree || (EmailInput && PasswordInput && LoginButton && GuestLoginButton))
	{
		return;
	}

	UBorder* RootPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LoginPanel"));
	LoginPanel = RootPanel;
	WidgetTree->RootWidget = RootPanel;

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NativeLoginRoot"));
	RootPanel->SetContent(RootBox);

	UTextBlock* NativeTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NativeLoginTitle"));
	NativeTitleText->SetText(NSLOCTEXT("DBALoginFlowWidget", "Title", "Divine Beasts Arena"));
	RootBox->AddChildToVerticalBox(NativeTitleText);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	RootBox->AddChildToVerticalBox(StatusText);

	ErrorText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ErrorText"));
	ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	RootBox->AddChildToVerticalBox(ErrorText);

	EmailInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("EmailInput"));
	EmailInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "EmailHint", "账号"));
	RootBox->AddChildToVerticalBox(EmailInput);

	PasswordInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("PasswordInput"));
	PasswordInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "PasswordHint", "密码"));
	PasswordInput->SetIsPassword(true);
	RootBox->AddChildToVerticalBox(PasswordInput);

	LoginButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LoginButton"));
	LoginButton->AddChild(MakeLoginButtonLabel(WidgetTree, NSLOCTEXT("DBALoginFlowWidget", "LoginButton", "登录")));
	RootBox->AddChildToVerticalBox(LoginButton);

	GuestLoginButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("GuestLoginButton"));
	GuestLoginButton->AddChild(MakeLoginButtonLabel(WidgetTree, NSLOCTEXT("DBALoginFlowWidget", "GuestButton", "游客登录")));
	RootBox->AddChildToVerticalBox(GuestLoginButton);
	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] 已创建原生回退登录布局。"));
}

void UDBALoginFlowWidgetBase::BindControls()
{
	if (LoginButton)
	{
		LoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleLoginClicked);
		LoginButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleLoginClicked);
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] 登录按钮为空，未绑定点击事件。"));
	}
	if (GuestLoginButton)
	{
		GuestLoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleGuestLoginClicked);
		GuestLoginButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleGuestLoginClicked);
		GuestLoginButton->SetIsEnabled(true);
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] 游客登录按钮为空，未绑定点击事件。"));
	}
	if (RememberToggleButton)
	{
		RememberToggleButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleRememberToggleClicked);
		RememberToggleButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleRememberToggleClicked);
	}
	if (AgreementToggleButton)
	{
		AgreementToggleButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleAgreementToggleClicked);
		AgreementToggleButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleAgreementToggleClicked);
	}
	if (PasswordVisibilityButton)
	{
		PasswordVisibilityButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandlePasswordVisibilityClicked);
		PasswordVisibilityButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandlePasswordVisibilityClicked);
	}
	if (ServerSelectButton)
	{
		ServerSelectButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleServerSelectClicked);
		ServerSelectButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleServerSelectClicked);
	}
	if (ForgotPasswordButton)
	{
		ForgotPasswordButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleForgotPasswordClicked);
		ForgotPasswordButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleForgotPasswordClicked);
	}
	if (RegisterAccountButton)
	{
		RegisterAccountButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleRegisterAccountClicked);
		RegisterAccountButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleRegisterAccountClicked);
	}
	if (AnnouncementButton)
	{
		AnnouncementButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleAnnouncementClicked);
		AnnouncementButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleAnnouncementClicked);
	}
	if (SupportButton)
	{
		SupportButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleSupportClicked);
		SupportButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleSupportClicked);
	}
	if (RepairButton)
	{
		RepairButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleRepairClicked);
		RepairButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleRepairClicked);
	}
	if (UserAgreementButton)
	{
		UserAgreementButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleUserAgreementClicked);
		UserAgreementButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleUserAgreementClicked);
	}
	if (PrivacyPolicyButton)
	{
		PrivacyPolicyButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandlePrivacyPolicyClicked);
		PrivacyPolicyButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandlePrivacyPolicyClicked);
	}
	UpdateReferenceToggleVisuals();
}
void UDBALoginFlowWidgetBase::UnbindControls()
{
	if (LoginButton)
	{
		LoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleLoginClicked);
	}
	if (GuestLoginButton)
	{
		GuestLoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleGuestLoginClicked);
	}
	if (RememberToggleButton)
	{
		RememberToggleButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleRememberToggleClicked);
	}
	if (AgreementToggleButton)
	{
		AgreementToggleButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleAgreementToggleClicked);
	}
	if (PasswordVisibilityButton)
	{
		PasswordVisibilityButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandlePasswordVisibilityClicked);
	}
	if (ServerSelectButton)
	{
		ServerSelectButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleServerSelectClicked);
	}
	if (ForgotPasswordButton)
	{
		ForgotPasswordButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleForgotPasswordClicked);
	}
	if (RegisterAccountButton)
	{
		RegisterAccountButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleRegisterAccountClicked);
	}
	if (AnnouncementButton)
	{
		AnnouncementButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleAnnouncementClicked);
	}
	if (SupportButton)
	{
		SupportButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleSupportClicked);
	}
	if (RepairButton)
	{
		RepairButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleRepairClicked);
	}
	if (UserAgreementButton)
	{
		UserAgreementButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleUserAgreementClicked);
	}
	if (PrivacyPolicyButton)
	{
		PrivacyPolicyButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandlePrivacyPolicyClicked);
	}
}

void UDBALoginFlowWidgetBase::SetStatus(const FText& InStatusText)
{
	if (StatusText)
	{
		StatusText->SetText(InStatusText);
		StatusText->SetVisibility(InStatusText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

UDBAFrontendFlowSubsystem* UDBALoginFlowWidgetBase::GetLoginFlow() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr;
}

UDBAFrontendFlowController* UDBALoginFlowWidgetBase::GetFrontendFlowController() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			return UIManager->GetFrontendFlowController();
		}
	}
	return nullptr;
}

void UDBALoginFlowWidgetBase::HandleBackgroundMusicFinished()
{
	BackgroundMusicComponent = nullptr;
}

void UDBALoginFlowWidgetBase::InitializeAudioAssets()
{
	if (!ButtonClickSound)
	{
		ButtonClickSound = LoadAssetIfCookedAvailable<USoundBase>(TEXT("/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick.SFX_UI_ButtonClick"));
		if (!ButtonClickSound)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] 未找到按钮点击音效。"));
		}
	}

	BackgroundMusicSound = nullptr;
}

void UDBALoginFlowWidgetBase::StartBackgroundMusic()
{
	StopBackgroundMusic();
}

void UDBALoginFlowWidgetBase::StopBackgroundMusic()
{
	if (!BackgroundMusicComponent)
	{
		return;
	}

	BackgroundMusicComponent->OnAudioFinished.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleBackgroundMusicFinished);
	BackgroundMusicComponent->Stop();
	BackgroundMusicComponent = nullptr;
}

void UDBALoginFlowWidgetBase::PlayButtonClickSfx() const
{
	if (ButtonClickSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ButtonClickSound, 0.85f);
	}
}

void UDBALoginFlowWidgetBase::InitializeVisualAssets()
{
	if (!LoginPanelTexture)
	{
		LoginPanelTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/Reference/T_DBA_LoginRef_Panel.T_DBA_LoginRef_Panel"));
		if (!LoginPanelTexture)
		{
			LoginPanelTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginPanel_StoneGold.T_DBA_LoginPanel_StoneGold"));
		}
	}

	if (!LoginButtonTexture)
	{
		LoginButtonTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/Reference/T_DBA_LoginRef_ButtonPrimary.T_DBA_LoginRef_ButtonPrimary"));
		if (!LoginButtonTexture)
		{
			LoginButtonTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginButton_ParchmentGold.T_DBA_LoginButton_ParchmentGold"));
		}
	}

	if (!ReferenceInputTexture)
	{
		ReferenceInputTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/Reference/T_DBA_LoginRef_Input.T_DBA_LoginRef_Input"));
	}
	if (!ReferenceFrameTexture)
	{
		ReferenceFrameTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/Reference/T_DBA_LoginRef_Frame.T_DBA_LoginRef_Frame"));
	}
	if (!ReferenceGuestButtonTexture)
	{
		ReferenceGuestButtonTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/Reference/T_DBA_LoginRef_ButtonGuest.T_DBA_LoginRef_ButtonGuest"));
	}
	if (!ReferenceSideToolTexture)
	{
		ReferenceSideToolTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/Reference/T_DBA_LoginRef_SideTool.T_DBA_LoginRef_SideTool"));
	}
	if (!ReferenceAgeBadgeTexture)
	{
		ReferenceAgeBadgeTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/Reference/T_DBA_LoginRef_AgeBadge.T_DBA_LoginRef_AgeBadge"));
	}
	if (!ReferenceCheckboxOnTexture)
	{
		ReferenceCheckboxOnTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/Reference/T_DBA_LoginRef_CheckOn.T_DBA_LoginRef_CheckOn"));
	}
	if (!ReferenceCheckboxOffTexture)
	{
		ReferenceCheckboxOffTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/Reference/T_DBA_LoginRef_CheckOff.T_DBA_LoginRef_CheckOff"));
	}

	if (UTexture2D* CustomLoginBackground = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginBackground_Custom.T_DBA_LoginBackground_Custom")))
	{
		LoginBackgroundTexture = CustomLoginBackground;
	}
	else if (!LoginBackgroundTexture)
	{
		LoginBackgroundTexture = LoadAssetIfCookedAvailable<UTexture2D>(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginForestSanctuary.T_DBA_LoginForestSanctuary"));
	}
}

void UDBALoginFlowWidgetBase::BuildReferenceNativeLayout()
{
	if (!WidgetTree)
	{
		return;
	}

	const FDBALoginVisualLayoutSpec Spec = GetReferenceVisualLayoutSpec();
	if (AvailableServers.IsEmpty())
	{
		AvailableServers = {
			FText::FromString(TEXT("苍穹之森")),
			FText::FromString(TEXT("星辉荒原")),
			FText::FromString(TEXT("青木幻林"))
		};
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ReferenceLoginRoot"));
	USizeBox* DesignRoot = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("LoginReferenceDesignRoot"));
	DesignRoot->SetWidthOverride(Spec.ReferenceDesignWidth);
	DesignRoot->SetHeightOverride(Spec.ReferenceDesignHeight);
	DesignRoot->SetContent(RootCanvas);
	WidgetTree->RootWidget = DesignRoot;

	const float InputBgWidth = Spec.PanelWidth - (Spec.PanelPadding * 2.0f) - 24.0f;

	UImage* BackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ReferenceForestBackground"));
	if (LoginBackgroundTexture)
	{
		ApplyFullBleedTextureToImage(BackgroundImage, LoginBackgroundTexture);
	}
	else
	{
		FSlateBrush BackgroundBrush;
		BackgroundBrush.DrawAs = ESlateBrushDrawType::Box;
		BackgroundBrush.TintColor = FSlateColor(FLinearColor(0.0f, 0.11f, 0.09f, 1.0f));
		BackgroundImage->SetBrush(BackgroundBrush);
		BackgroundImage->SetColorAndOpacity(FLinearColor(0.0f, 0.11f, 0.09f, 1.0f));
	}
	AddCanvasChildAnchored(
		RootCanvas,
		BackgroundImage,
		FAnchors(0.0f, 0.0f, 1.0f, 1.0f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f));

	UImage* Vignette = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ReferenceVignette"));
	FSlateBrush VignetteBrush;
	VignetteBrush.DrawAs = ESlateBrushDrawType::Box;
	VignetteBrush.TintColor = FSlateColor(FLinearColor(0.0f, 0.02f, 0.015f, 0.38f));
	Vignette->SetBrush(VignetteBrush);
	Vignette->SetColorAndOpacity(FLinearColor(0.0f, 0.02f, 0.015f, 0.38f));
	AddCanvasChildAnchored(
		RootCanvas,
		Vignette,
		FAnchors(0.0f, 0.0f, 1.0f, 1.0f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f));

	if (ReferenceFrameTexture)
	{
		UImage* FrameImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ReferenceOuterFrame"));
		FrameImage->SetBrushFromTexture(ReferenceFrameTexture);
		FrameImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.94f));
		AddCanvasChildAnchored(RootCanvas, FrameImage, FAnchors(0.0f, 0.0f, 1.0f, 1.0f), FMargin(0.0f));
	}

	TitleText = MakeText(WidgetTree, TEXT("ReferenceLoginTitle"), Spec.TitleText, 92.0f, FLinearColor(1.0f, 0.78f, 0.34f, 1.0f));
	AddCanvasChildAnchored(
		RootCanvas,
		TitleText,
		FAnchors(0.5f, 0.0f, 0.5f, 0.0f),
		FMargin(0.0f, 86.0f, 760.0f, 132.0f),
		FVector2D(0.5f, 0.0f));

	UTextBlock* SubtitleText = MakeText(WidgetTree, TEXT("ReferenceLoginSubtitle"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceSubtitle", "Divine Beasts Arena"), 26.0f, FLinearColor(0.38f, 0.76f, 1.0f, 0.86f));
	AddCanvasChildAnchored(
		RootCanvas,
		SubtitleText,
		FAnchors(0.5f, 0.0f, 0.5f, 0.0f),
		FMargin(0.0f, 205.0f, 420.0f, 42.0f),
		FVector2D(0.5f, 0.0f));

	UVerticalBox* SideTools = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ReferenceSideTools"));
	AddCanvasChildAnchored(
		RootCanvas,
		SideTools,
		FAnchors(1.0f, 0.5f, 1.0f, 0.5f),
		FMargin(-154.0f, -170.0f, 108.0f, 360.0f));
	for (int32 Index = 0; Index < Spec.LeftToolLabels.Num(); ++Index)
	{
		const FText ButtonLabel = FText::Format(NSLOCTEXT("DBALoginFlowWidget", "SideToolButtonFormat", "{0}\n{1}"),
			Index == 0 ? FText::FromString(TEXT("!")) : (Index == 1 ? FText::FromString(TEXT("☎")) : FText::FromString(TEXT("×"))),
			Spec.LeftToolLabels[Index]);
		UButton* ToolButton = MakeTextButton(WidgetTree, FName(*FString::Printf(TEXT("ReferenceSideTool_%d"), Index)), ButtonLabel, 22.0f, GoldText);
		if (ReferenceSideToolTexture)
		{
			FSlateBrush ToolBrush;
			ToolBrush.SetResourceObject(ReferenceSideToolTexture);
			ToolBrush.ImageSize = FVector2D(96.0f, 96.0f);
			ToolBrush.DrawAs = ESlateBrushDrawType::Image;
			FButtonStyle ToolStyle = ToolButton->GetStyle();
			ToolStyle.SetNormal(ToolBrush);
			ToolStyle.SetHovered(ToolBrush);
			ToolStyle.SetPressed(ToolBrush);
			ToolButton->SetStyle(ToolStyle);
		}
		if (Index == 0)
		{
			AnnouncementButton = ToolButton;
		}
		else if (Index == 1)
		{
			SupportButton = ToolButton;
		}
		else
		{
			RepairButton = ToolButton;
		}
		AddVerticalChild(SideTools, ToolButton, FMargin(0.0f, 0.0f, 0.0f, 22.0f));
	}

	UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LoginPanel"));
	LoginPanel = PanelBorder;
	AddCanvasChildAnchored(
		RootCanvas,
		PanelBorder,
		FAnchors(Spec.PanelAnchorX, 0.0f, Spec.PanelAnchorX, 0.0f),
		FMargin(0.0f, Spec.PanelTopOffset, Spec.PanelWidth, Spec.PanelHeight),
		FVector2D(0.5f, 0.0f));
	if (LoginPanelTexture)
	{
		PanelBorder->SetBrushFromTexture(LoginPanelTexture);
		PanelBorder->SetBrushColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.97f));
	}
	else
	{
		PanelBorder->SetBrushColor(DeepJade);
	}
	PanelBorder->SetPadding(FMargin(Spec.PanelPadding, Spec.PanelPadding, Spec.PanelPadding, Spec.PanelPadding * 0.72f));

	UCanvasPanel* FormCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ReferenceLoginForm"));
	PanelBorder->SetContent(FormCanvas);

	UTextBlock* ServerLabel = MakeText(WidgetTree, TEXT("ReferenceServerLabel"), FText::FromString(TEXT("服务器:")), 24.0f, FLinearColor(0.96f, 0.90f, 0.76f, 1.0f), ETextJustify::Right);
	AddCanvasChild(FormCanvas, ServerLabel, FVector2D(10.0f, 3.0f), FVector2D(118.0f, 48.0f));
	ServerSelectButton = MakeTextButton(WidgetTree, TEXT("ServerSelectButton"), FText::GetEmpty(), 23.0f, GoldText);
	ServerNameText = Cast<UTextBlock>(ServerSelectButton->GetContent());
	AddCanvasChild(FormCanvas, ServerSelectButton, FVector2D(160.0f, 0.0f), FVector2D(322.0f, 56.0f));
	UTextBlock* ServerQualityText = MakeText(WidgetTree, TEXT("ReferenceServerQuality"), FText::FromString(TEXT("●  流畅")), 21.0f, FLinearColor(0.42f, 1.0f, 0.28f, 1.0f), ETextJustify::Left);
	AddCanvasChild(FormCanvas, ServerQualityText, FVector2D(506.0f, 12.0f), FVector2D(120.0f, 40.0f));

	UImage* AccountBg = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ReferenceAccountInputBg"));
	if (ReferenceInputTexture)
	{
		AccountBg->SetBrushFromTexture(ReferenceInputTexture);
	}
	AddCanvasChild(FormCanvas, AccountBg, FVector2D(20.0f, 80.0f), FVector2D(InputBgWidth, Spec.InputRowHeight));
	UTextBlock* AccountLabel = MakeText(WidgetTree, TEXT("ReferenceAccountLabel"), FText::FromString(TEXT("♟  账号")), 23.0f, FLinearColor(0.98f, 0.86f, 0.58f, 1.0f), ETextJustify::Left);
	AddCanvasChild(FormCanvas, AccountLabel, FVector2D(36.0f, 94.0f), FVector2D(132.0f, 36.0f));
	EmailInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("EmailInput"));
	EmailInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "AccountHint", "\u8bf7\u8f93\u5165\u8d26\u53f7"));
	ApplyEditableBoxStyle(EmailInput, Spec.InputMinDesiredWidth);
	AddCanvasChild(FormCanvas, EmailInput, FVector2D(200.0f, 86.0f), FVector2D(520.0f, Spec.InputEditableHeight));

	UImage* PasswordBg = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ReferencePasswordInputBg"));
	if (ReferenceInputTexture)
	{
		PasswordBg->SetBrushFromTexture(ReferenceInputTexture);
	}
	AddCanvasChild(FormCanvas, PasswordBg, FVector2D(20.0f, 166.0f), FVector2D(InputBgWidth, Spec.InputRowHeight));
	UTextBlock* PasswordLabel = MakeText(WidgetTree, TEXT("ReferencePasswordLabel"), FText::FromString(TEXT("▣  密码")), 23.0f, FLinearColor(0.98f, 0.86f, 0.58f, 1.0f), ETextJustify::Left);
	AddCanvasChild(FormCanvas, PasswordLabel, FVector2D(36.0f, 180.0f), FVector2D(132.0f, 36.0f));
	PasswordInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("PasswordInput"));
	PasswordInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "ReferencePasswordHint", "\u8bf7\u8f93\u5165\u5bc6\u7801"));
	PasswordInput->SetIsPassword(true);
	ApplyEditableBoxStyle(PasswordInput, Spec.InputMinDesiredWidth);
	AddCanvasChild(FormCanvas, PasswordInput, FVector2D(200.0f, 172.0f), FVector2D(430.0f, Spec.InputEditableHeight));
	PasswordVisibilityButton = MakeTextButton(WidgetTree, TEXT("PasswordVisibilityButton"), FText::FromString(TEXT("◉")), 24.0f, FLinearColor(0.96f, 0.79f, 0.45f, 1.0f));
	PasswordEyeText = Cast<UTextBlock>(PasswordVisibilityButton->GetContent());
	AddCanvasChild(FormCanvas, PasswordVisibilityButton, FVector2D(648.0f, 176.0f), FVector2D(64.0f, 48.0f));

	RememberToggleButton = MakeTextButton(WidgetTree, TEXT("RememberToggleButton"), FText::GetEmpty(), 24.0f, FLinearColor::White);
	RememberCheckText = Cast<UTextBlock>(RememberToggleButton->GetContent());
	AddCanvasChild(FormCanvas, RememberToggleButton, FVector2D(20.0f, 246.0f), FVector2D(42.0f, 42.0f));
	UTextBlock* RememberLabel = MakeText(WidgetTree, TEXT("ReferenceRememberText"), FText::FromString(TEXT("记住我")), 21.0f, GoldText, ETextJustify::Left);
	AddCanvasChild(FormCanvas, RememberLabel, FVector2D(72.0f, 250.0f), FVector2D(120.0f, 34.0f));
	ForgotPasswordButton = MakeTextButton(WidgetTree, TEXT("ForgotPasswordButton"), FText::FromString(TEXT("忘记密码")), 21.0f, GoldText);
	AddCanvasChild(FormCanvas, ForgotPasswordButton, FVector2D(508.0f, 244.0f), FVector2D(128.0f, 42.0f));

	LoginButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LoginButton"));
	LoginButton->AddChild(MakeLoginButtonLabel(WidgetTree, Spec.PrimaryButtonText));
	AddCanvasChild(FormCanvas, LoginButton, FVector2D(96.0f, 296.0f), FVector2D(472.0f, 86.0f));

	GuestLoginButton = MakeTextButton(WidgetTree, TEXT("GuestLoginButton"), FText::FromString(TEXT("游客登录")), 30.0f, FLinearColor(0.88f, 0.92f, 1.0f, 1.0f));
	AddCanvasChild(FormCanvas, GuestLoginButton, FVector2D(174.0f, 394.0f), FVector2D(316.0f, 66.0f));
	RegisterAccountButton = MakeTextButton(WidgetTree, TEXT("RegisterAccountButton"), FText::FromString(TEXT("注册账号")), 23.0f, GoldText);
	AddCanvasChild(FormCanvas, RegisterAccountButton, FVector2D(226.0f, 468.0f), FVector2D(212.0f, 46.0f));

	ErrorText = MakeText(WidgetTree, TEXT("ErrorText"), FText::GetEmpty(), 18.0f, FLinearColor(1.0f, 0.35f, 0.24f, 1.0f));
	ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	AddCanvasChild(FormCanvas, ErrorText, FVector2D(34.0f, 512.0f), FVector2D(596.0f, 28.0f));
	StatusText = MakeText(WidgetTree, TEXT("StatusText"), FText::GetEmpty(), 18.0f, FLinearColor(0.76f, 0.96f, 0.74f, 1.0f));
	StatusText->SetVisibility(ESlateVisibility::Collapsed);
	AddCanvasChild(FormCanvas, StatusText, FVector2D(34.0f, 536.0f), FVector2D(596.0f, 28.0f));

	UHorizontalBox* AgreementRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ReferenceAgreementRow"));
	AddCanvasChildAnchored(
		RootCanvas,
		AgreementRow,
		FAnchors(0.5f, 1.0f, 0.5f, 1.0f),
		FMargin(0.0f, -102.0f, 650.0f, 48.0f),
		FVector2D(0.5f, 0.0f));
	AgreementToggleButton = MakeTextButton(WidgetTree, TEXT("AgreementToggleButton"), FText::GetEmpty(), 24.0f, FLinearColor::White);
	AgreementCheckText = Cast<UTextBlock>(AgreementToggleButton->GetContent());
	if (UHorizontalBoxSlot* AgreementCheckSlot = AgreementRow->AddChildToHorizontalBox(AgreementToggleButton))
	{
		AgreementCheckSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}
	AgreementRow->AddChildToHorizontalBox(MakeText(WidgetTree, TEXT("ReferenceAgreementLead"), FText::FromString(TEXT("我已详细阅读并同意")), 22.0f, FLinearColor(0.92f, 0.88f, 0.74f, 1.0f), ETextJustify::Left));
	UserAgreementButton = MakeTextButton(WidgetTree, TEXT("UserAgreementButton"), FText::FromString(TEXT("《用户协议》")), 22.0f, FLinearColor(0.42f, 0.84f, 1.0f, 1.0f));
	AgreementRow->AddChildToHorizontalBox(UserAgreementButton);
	AgreementRow->AddChildToHorizontalBox(MakeText(WidgetTree, TEXT("ReferenceAgreementAnd"), FText::FromString(TEXT("和")), 22.0f, FLinearColor(0.92f, 0.88f, 0.74f, 1.0f), ETextJustify::Left));
	PrivacyPolicyButton = MakeTextButton(WidgetTree, TEXT("PrivacyPolicyButton"), FText::FromString(TEXT("《隐私政策》")), 22.0f, FLinearColor(0.42f, 0.84f, 1.0f, 1.0f));
	AgreementRow->AddChildToHorizontalBox(PrivacyPolicyButton);

	if (ReferenceAgeBadgeTexture)
	{
		UImage* AgeBadge = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ReferenceAgeBadge"));
		AgeBadge->SetBrushFromTexture(ReferenceAgeBadgeTexture);
		AddCanvasChildAnchored(
			RootCanvas,
			AgeBadge,
			FAnchors(0.0f, 1.0f, 0.0f, 1.0f),
			FMargin(54.0f, -190.0f, 104.0f, 130.0f));
	}

	UpdateReferenceToggleVisuals();
}

bool UDBALoginFlowWidgetBase::CanSubmitLoginAction()
{
	if (!bAgreementAccepted)
	{
		ShowError(TEXT("请先阅读并同意《用户协议》和《隐私政策》。"));
		return false;
	}
	return true;
}

void UDBALoginFlowWidgetBase::UpdateReferenceToggleVisuals()
{
	const auto ApplyCheckboxStyle = [this](UButton* Button, const bool bChecked)
	{
		if (!Button)
		{
			return;
		}

		if (UTexture2D* Texture = bChecked ? ReferenceCheckboxOnTexture : ReferenceCheckboxOffTexture)
		{
			FSlateBrush Brush;
			Brush.SetResourceObject(Texture);
			Brush.ImageSize = FVector2D(40.0f, 40.0f);
			Brush.DrawAs = ESlateBrushDrawType::Image;

			FButtonStyle Style = Button->GetStyle();
			Style.SetNormal(Brush);
			Style.SetHovered(Brush);
			Style.SetPressed(Brush);
			Button->SetStyle(Style);
		}
	};

	if (RememberCheckText)
	{
		RememberCheckText->SetText(bRememberAccount ? FText::FromString(TEXT("✓")) : FText::GetEmpty());
	}
	if (AgreementCheckText)
	{
		AgreementCheckText->SetText(bAgreementAccepted ? FText::FromString(TEXT("✓")) : FText::GetEmpty());
	}
	ApplyCheckboxStyle(RememberToggleButton, bRememberAccount);
	ApplyCheckboxStyle(AgreementToggleButton, bAgreementAccepted);

	if (PasswordInput)
	{
		PasswordInput->SetIsPassword(!bPasswordVisible);
	}
	if (PasswordEyeText)
	{
		PasswordEyeText->SetText(bPasswordVisible ? FText::FromString(TEXT("◌")) : FText::FromString(TEXT("◉")));
	}
	if (ServerNameText && AvailableServers.IsValidIndex(SelectedServerIndex))
	{
		ServerNameText->SetText(FText::Format(FText::FromString(TEXT("{0}        ▼")), AvailableServers[SelectedServerIndex]));
	}
}

void UDBALoginFlowWidgetBase::ApplyLoginBackgroundTexture()
{
	if (!WidgetTree || !LoginBackgroundTexture)
	{
		return;
	}

	const auto ApplyTextureToImage = [this](UImage* Image)
	{
		ApplyFullBleedTextureToImage(Image, LoginBackgroundTexture);
	};

	if (UWidget* BackgroundWidget = FindWidgetByNames(WidgetTree, {
		TEXT("ForestBackgroundImage"),
		TEXT("LoginBackgroundImage"),
		TEXT("BackgroundImage"),
		TEXT("ReferenceForestBackground")
	}))
	{
		if (UImage* BackgroundImage = Cast<UImage>(BackgroundWidget))
		{
			ApplyTextureToImage(BackgroundImage);
			return;
		}

		if (UBorder* BackgroundBorder = Cast<UBorder>(BackgroundWidget))
		{
			ApplyFullBleedTextureToBorder(BackgroundBorder, LoginBackgroundTexture);
			return;
		}
	}

	TArray<UWidget*> AllWidgets;
	WidgetTree->GetAllWidgets(AllWidgets);
	for (UWidget* Widget : AllWidgets)
	{
		UImage* Image = Cast<UImage>(Widget);
		if (!Image)
		{
			continue;
		}

		const FString WidgetName = Image->GetName();
		if (WidgetName.Contains(TEXT("Background"), ESearchCase::IgnoreCase)
			&& !WidgetName.Contains(TEXT("Vignette"), ESearchCase::IgnoreCase)
			&& !WidgetName.Contains(TEXT("Panel"), ESearchCase::IgnoreCase)
			&& !WidgetName.Contains(TEXT("Button"), ESearchCase::IgnoreCase))
		{
			ApplyTextureToImage(Image);
			return;
		}
	}

	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return;
	}

	UImage* InjectedBackground = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("RuntimeLoginBackgroundImage"));
	ApplyTextureToImage(InjectedBackground);
	if (UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(InjectedBackground))
	{
		BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BackgroundSlot->SetOffsets(FMargin(0.0f));
		BackgroundSlot->SetZOrder(-2000);
	}
}

void UDBALoginFlowWidgetBase::ApplyVisualStyle()
{
	ApplyLoginBackgroundTexture();

	if (bUseComposedImageLayout)
	{
		if (UBorder* ComposedPanel = Cast<UBorder>(LoginPanel))
		{
			FSlateBrush EmptyPanelBrush;
			EmptyPanelBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
			ComposedPanel->SetBrush(EmptyPanelBrush);
			ComposedPanel->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
			ComposedPanel->SetPadding(FMargin(0.0f));
		}

		const auto HideComposedLegacyWidget = [this](const TArray<FName>& Names)
		{
			if (UWidget* Widget = FindWidgetByNames(WidgetTree, Names))
			{
				Widget->SetVisibility(ESlateVisibility::Collapsed);
			}
		};

		HideComposedLegacyWidget({
			TEXT("DarkVignetteOverlay"), TEXT("FrameTopLine"), TEXT("FrameBottomLine"),
			TEXT("FrameLeftLine"), TEXT("FrameRightLine"), TEXT("TitleBackplate"), TEXT("TitleText"),
			TEXT("ServerLabelText"), TEXT("ServerButton"), TEXT("ServerArrowText"), TEXT("ServerStatusText"),
			TEXT("AccountRowFrame"), TEXT("AccountIconText"), TEXT("AccountLabelText"),
			TEXT("PasswordRowFrame"), TEXT("PasswordIconText"), TEXT("PasswordLabelText"),
			TEXT("PasswordEyeText"), TEXT("RememberText"), TEXT("ForgotPasswordText"),
			TEXT("LoginButtonLabel"), TEXT("GuestLoginButtonLabel"), TEXT("RegisterAccountText"),
			TEXT("NoticeToolLabel"), TEXT("SupportToolLabel"), TEXT("RepairToolLabel"),
			TEXT("AgeRatingPanel"), TEXT("AgreementText"), TEXT("RegisterAccountButtonLabel"),
			TEXT("AccountCenterButtonLabel"), TEXT("OfficialWebsiteButtonLabel"),
			TEXT("PlayerCommunityButtonLabel"), TEXT("ExitGameButtonLabel")
		});

		ApplyComposedInputStyle(EmailInput);
		ApplyComposedInputStyle(PasswordInput);

		TArray<UWidget*> ComposedWidgets;
		WidgetTree->GetAllWidgets(ComposedWidgets);
		for (UWidget* Widget : ComposedWidgets)
		{
			UButton* Button = Cast<UButton>(Widget);
			if (!Button)
			{
				continue;
			}

			FSlateBrush TransparentBrush;
			TransparentBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
			FButtonStyle TransparentStyle = Button->GetStyle();
			TransparentStyle.SetNormal(TransparentBrush);
			TransparentStyle.SetHovered(TransparentBrush);
			TransparentStyle.SetPressed(TransparentBrush);
			TransparentStyle.SetDisabled(TransparentBrush);
			TransparentStyle.SetNormalPadding(FMargin(0.0f));
			TransparentStyle.SetPressedPadding(FMargin(0.0f));
			Button->SetStyle(TransparentStyle);
		}
		return;
	}

	if (!LoginPanel)
	{
		LoginPanel = FindLoginPanelWidget(WidgetTree);
	}

	if (LoginPanelTexture && LoginPanel)
	{
		if (UBorder* PanelBorder = Cast<UBorder>(LoginPanel))
		{
			PanelBorder->SetBrushFromTexture(LoginPanelTexture);
			PanelBorder->SetBrushColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.96f));
		}
		else if (UImage* PanelImage = Cast<UImage>(LoginPanel))
		{
			PanelImage->SetBrushFromTexture(LoginPanelTexture);
			PanelImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.96f));
		}
	}

	ApplyButtonTextureStyle(LoginButton);
	ApplyGuestButtonStyle(GuestLoginButton);
	ApplyEditableBoxStyle(EmailInput, GetReferenceVisualLayoutSpec().InputMinDesiredWidth);
	ApplyEditableBoxStyle(PasswordInput, GetReferenceVisualLayoutSpec().InputMinDesiredWidth);
}

void UDBALoginFlowWidgetBase::ApplyComposedInputStyle(UEditableTextBox* TextBox) const
{
	if (!TextBox)
	{
		return;
	}

	// The composed-image widget owns the Slate style. C++ only keeps its sizing
	// contract here; changing FEditableTextBoxStyle after Slate construction can
	// invalidate the font attribute in UE 5.8 preview rendering.
	TextBox->SetMinDesiredWidth(1.0f);
}

void UDBALoginFlowWidgetBase::ApplyButtonTextureStyle(UButton* Button) const
{
	if (!Button || !LoginButtonTexture)
	{
		return;
	}

	FSlateBrush NormalBrush;
	NormalBrush.SetResourceObject(LoginButtonTexture);
	NormalBrush.ImageSize = FVector2D(512.0f, 160.0f);
	NormalBrush.DrawAs = ESlateBrushDrawType::Image;
	NormalBrush.TintColor = FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));

	FSlateBrush HoveredBrush = NormalBrush;
	HoveredBrush.TintColor = FSlateColor(FLinearColor(1.18f, 1.08f, 0.82f, 1.0f));

	FSlateBrush PressedBrush = NormalBrush;
	PressedBrush.TintColor = FSlateColor(FLinearColor(0.78f, 0.65f, 0.44f, 1.0f));

	FSlateBrush DisabledBrush = NormalBrush;
	DisabledBrush.TintColor = FSlateColor(FLinearColor(0.38f, 0.34f, 0.28f, 0.85f));

	FButtonStyle Style = Button->GetStyle();
	Style.SetNormal(NormalBrush);
	Style.SetHovered(HoveredBrush);
	Style.SetPressed(PressedBrush);
	Style.SetDisabled(DisabledBrush);
	Style.SetNormalPadding(FMargin(2.0f));
	Style.SetPressedPadding(FMargin(3.0f, 4.0f, 1.0f, 0.0f));
	Button->SetStyle(Style);
}

void UDBALoginFlowWidgetBase::UpdateLoadingStateByFlow(EDBALoginFlowState NewState)
{
	const bool bIsLoading = NewState == EDBALoginFlowState::Authenticating || NewState == EDBALoginFlowState::LoadingCharacters;
	if (LoginButton)
	{
		LoginButton->SetIsEnabled(!bIsLoading);
	}
	if (GuestLoginButton)
	{
		GuestLoginButton->SetIsEnabled(!bIsLoading);
	}
}

void UDBALoginFlowWidgetBase::ApplyGuestButtonStyle(UButton* Button) const
{
	if (!Button)
	{
		return;
	}

	FSlateBrush NormalBrush;
	if (ReferenceGuestButtonTexture)
	{
		NormalBrush.SetResourceObject(ReferenceGuestButtonTexture);
		NormalBrush.ImageSize = FVector2D(360.0f, 72.0f);
		NormalBrush.DrawAs = ESlateBrushDrawType::Image;
		NormalBrush.TintColor = FSlateColor(FLinearColor::White);
	}
	else
	{
		NormalBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
		NormalBrush.TintColor = FSlateColor(FLinearColor(0.05f, 0.28f, 0.62f, 0.96f));
	}

	FSlateBrush HoveredBrush = NormalBrush;
	HoveredBrush.TintColor = FSlateColor(FLinearColor(0.08f, 0.40f, 0.84f, 1.0f));

	FSlateBrush PressedBrush = NormalBrush;
	PressedBrush.TintColor = FSlateColor(FLinearColor(0.03f, 0.18f, 0.42f, 1.0f));

	FSlateBrush DisabledBrush = NormalBrush;
	DisabledBrush.TintColor = FSlateColor(FLinearColor(0.03f, 0.10f, 0.20f, 0.72f));

	FButtonStyle Style = Button->GetStyle();
	Style.SetNormal(NormalBrush);
	Style.SetHovered(HoveredBrush);
	Style.SetPressed(PressedBrush);
	Style.SetDisabled(DisabledBrush);
	Style.SetNormalPadding(FMargin(2.0f));
	Style.SetPressedPadding(FMargin(3.0f, 4.0f, 1.0f, 0.0f));
	Button->SetStyle(Style);
}
