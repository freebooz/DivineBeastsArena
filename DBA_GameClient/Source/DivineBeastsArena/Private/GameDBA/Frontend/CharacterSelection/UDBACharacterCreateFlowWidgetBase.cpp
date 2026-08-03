// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Frontend/CharacterSelection/UDBACharacterCreateFlowWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/AudioComponent.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ContentWidget.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameDBA/Frontend/DBAFrontendEnvironmentSubsystem.h"
#include "GameDBA/UI/Controllers/DBAGameUIManager.h"
#include "GameDBA/UI/Frontend/DBAFrontendFlowController.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Data/Assets/DBAZodiacHeroDataAsset.h"
#include "GameDBA/UI/DBAUIDeveloperSettings.h"
#include "GameDBA/UI/DBAUIFontUtils.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterPresentationActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace
{
	template <typename EnumType>
	FText EnumValueText(EnumType Value)
	{
		if (const UEnum* Enum = StaticEnum<EnumType>())
		{
			return Enum->GetDisplayNameTextByValue(static_cast<int64>(Value));
		}
		return NSLOCTEXT("DBACharacterCreateWidget", "UnknownEnum", "未知");
	}

	UTextBlock* MakeCreateButtonLabel(UWidgetTree* WidgetTree, const FText& Label)
	{
		UTextBlock* TextBlock = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()) : nullptr;
		if (TextBlock)
		{
			TextBlock->SetText(Label);
		}
		return TextBlock;
	}

	UTextBlock* GetCreateButtonLabel(UButton* Button)
	{
		if (const UContentWidget* ContentHost = Cast<UContentWidget>(Button))
		{
			return Cast<UTextBlock>(ContentHost->GetContent());
		}
		return nullptr;
	}

	void SetCreateButtonLabel(UButton* Button, const FText& Label)
	{
		if (UTextBlock* TextBlock = GetCreateButtonLabel(Button))
		{
			TextBlock->SetText(Label);
		}
	}

	void SetCreateTextBlockByNames(UWidgetTree* WidgetTree, const TArray<FName>& Names, const FText& Text)
	{
		if (!WidgetTree)
		{
			return;
		}

		for (const FName& Name : Names)
		{
			if (UTextBlock* TextBlock = Cast<UTextBlock>(WidgetTree->FindWidget(Name)))
			{
				TextBlock->SetText(Text);
			}
		}
	}

	void ReplaceCreateTextBlockValue(UWidgetTree* WidgetTree, const FString& EnglishText, const FText& ChineseText)
	{
		if (!WidgetTree)
		{
			return;
		}

		WidgetTree->ForEachWidgetAndDescendants(
			[&EnglishText, &ChineseText](UWidget* Widget)
			{
				UTextBlock* TextBlock = Cast<UTextBlock>(Widget);
				if (TextBlock && TextBlock->GetText().ToString().Equals(EnglishText, ESearchCase::IgnoreCase))
				{
					TextBlock->SetText(ChineseText);
				}
			});
	}

	template <typename EnumType>
	EnumType CycleEnumValue(EnumType Current, const TArray<EnumType>& Values)
	{
		const int32 CurrentIndex = Values.IndexOfByKey(Current);
		return Values.IsValidIndex(CurrentIndex + 1) ? Values[CurrentIndex + 1] : Values[0];
	}

	bool IsBlockedCharacterCreateNavigationKey(const FKey& Key)
	{
		return Key == EKeys::Left || Key == EKeys::Right;
	}

	bool IsCreateUiChromeWidgetName(const FString& Name)
	{
		const FString LowerName = Name.ToLower();
		return LowerName.Contains(TEXT("button"))
			|| LowerName.Contains(TEXT("title"))
			|| LowerName.Contains(TEXT("validation"))
			|| LowerName.Contains(TEXT("hint"))
			|| LowerName.Contains(TEXT("input"))
			|| LowerName.Contains(TEXT("zodiac"))
			|| LowerName.Contains(TEXT("element"))
			|| LowerName.Contains(TEXT("fivecamp"))
			|| LowerName.Contains(TEXT("create"))
			|| LowerName.Contains(TEXT("back"))
			|| LowerName.Contains(TEXT("label"))
			|| LowerName.Contains(TEXT("text"));
	}

	bool IsCreateBackgroundImageName(const FString& Name)
	{
		const FString LowerName = Name.ToLower();
		return LowerName.Contains(TEXT("previewstage"))
			|| LowerName.Contains(TEXT("characterpreviewbg"))
			|| LowerName.Contains(TEXT("characterpreviewbackdrop"))
			|| LowerName.Contains(TEXT("characterpreviewforeground"))
			|| LowerName.Contains(TEXT("autobackground"))
			|| LowerName.Contains(TEXT("background"))
			|| LowerName.Contains(TEXT("backdrop"))
			|| LowerName.Contains(TEXT("bgimage"))
			|| LowerName.Contains(TEXT("bg_"))
			|| LowerName.EndsWith(TEXT("_bg"))
			|| LowerName.Contains(TEXT("fullscreen"))
			|| LowerName.Contains(TEXT("dimmer"))
			|| LowerName.Contains(TEXT("veil"))
			|| LowerName.Contains(TEXT("overlaybg"));
	}

	bool IsCreateFullBleedCanvasSlot(const UWidget* Widget)
	{
		const UCanvasPanelSlot* CanvasSlot = Widget ? Cast<UCanvasPanelSlot>(Widget->Slot) : nullptr;
		if (!CanvasSlot)
		{
			return false;
		}

		const FAnchors Anchors = CanvasSlot->GetAnchors();
		const FMargin Offsets = CanvasSlot->GetOffsets();
		const bool bStretched = FMath::IsNearlyEqual(Anchors.Minimum.X, 0.0f)
			&& FMath::IsNearlyEqual(Anchors.Minimum.Y, 0.0f)
			&& FMath::IsNearlyEqual(Anchors.Maximum.X, 1.0f)
			&& FMath::IsNearlyEqual(Anchors.Maximum.Y, 1.0f);
		const bool bNearZeroOffsets = FMath::Abs(Offsets.Left) <= 8.0f
			&& FMath::Abs(Offsets.Top) <= 8.0f
			&& FMath::Abs(Offsets.Right) <= 8.0f
			&& FMath::Abs(Offsets.Bottom) <= 8.0f;
		return bStretched && bNearZeroOffsets;
	}

	void ApplyWidgetSlot(UWidget* Widget, const FAnchors& Anchors, const FMargin& Offsets, const FVector2D& Alignment)
	{
		if (!Widget)
		{
			return;
		}

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
		{
			CanvasSlot->SetAnchors(Anchors);
			CanvasSlot->SetOffsets(Offsets);
			CanvasSlot->SetAlignment(Alignment);
			CanvasSlot->SetAutoSize(false);
		}
	}

	void HideCreateWorldStageBackgroundImages(UWidgetTree* WidgetTree)
	{
		if (!WidgetTree)
		{
			return;
		}

		int32 ClearedCount = 0;
		WidgetTree->ForEachWidgetAndDescendants(
			[&ClearedCount](UWidget* Widget)
			{
				if (!Widget)
				{
					return;
				}

				const FString WidgetName = Widget->GetName();
				if (IsCreateUiChromeWidgetName(WidgetName))
				{
					return;
				}

				const bool bNamedBackground = IsCreateBackgroundImageName(WidgetName);
				const bool bFullBleed = IsCreateFullBleedCanvasSlot(Widget);
				if (!bNamedBackground && !bFullBleed)
				{
					return;
				}

				if (UImage* Image = Cast<UImage>(Widget))
				{
					Image->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
					Image->SetRenderOpacity(0.0f);
					Image->SetVisibility(ESlateVisibility::HitTestInvisible);
					++ClearedCount;
					return;
				}

				if (UBorder* Border = Cast<UBorder>(Widget))
				{
					FLinearColor Color = Border->GetBrushColor();
					Color.A = 0.0f;
					Border->SetBrushColor(Color);
					Border->SetRenderOpacity(1.0f);
					Border->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
					++ClearedCount;
				}
			});

		UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 已透明化遮挡世界舞台的背景控件数量=%d"), ClearedCount);
	}

	void ApplyCreateMenuInputMode(UUserWidget* Widget)
	{
		if (!Widget)
		{
			return;
		}

		APlayerController* PC = Widget->GetOwningPlayer();
		if (!PC && Widget->GetWorld())
		{
			PC = Widget->GetWorld()->GetFirstPlayerController();
		}
		if (!PC)
		{
			return;
		}

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		PC->SetInputMode(InputMode);
		PC->CurrentMouseCursor = EMouseCursor::Default;
		PC->DefaultMouseCursor = EMouseCursor::Default;
		PC->SetShowMouseCursor(true);
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		Widget->SetFocus();
	}
}

UDBACharacterCreateFlowWidgetBase::UDBACharacterCreateFlowWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	// 创建界面需要透视世界 3D 舞台（角色/灯光/地板），禁止基类注入不透明背景。
	bAutoInjectBackground = false;
	BackgroundOpacity = 0.0f;
}

void UDBACharacterCreateFlowWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBACharacterCreateFlowWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	EnsureNativeFallbackLayout();
	ResolveBoundWidgetsFromWidgetTree();
	ApplyBlueprintLayoutOverrides();
	ApplyLocalizedText();
	BindButtonClickAudio();
	BindControls();
	InitializeAudioAssets();
	DBAUIFonts::ApplyGameFontToWidgetTree(WidgetTree);
	ApplyCreateMenuInputMode(this);
	InitializePresentationLevel();
	InitializeZodiacPresentationData();
	if (CharacterNameInput)
	{
		CharacterNameInput->SetIsEnabled(true);
		CharacterNameInput->SetIsReadOnly(false);
		CharacterNameInput->SetKeyboardFocus();
	}
	else if (CharacterNameEditableText)
	{
		CharacterNameEditableText->SetIsEnabled(true);
		CharacterNameEditableText->SetIsReadOnly(false);
		CharacterNameEditableText->SetKeyboardFocus();
	}
	RefreshChoiceText();
	Validate();
	bIsSubmittingCreate = false;
	if (CreateButton)
	{
		CreateButton->SetIsEnabled(true);
	}

	if (UDBAFrontendFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleFlowError);
		LoginFlow->OnFlowError.AddDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleFlowError);
	}
}

void UDBACharacterCreateFlowWidgetBase::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeferredPresentationActivateTimerHandle);
	}
	DeferredPresentationActivateRetryCount = 0;

	if (UDBAFrontendFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleFlowError);
	}

	UnbindControls();
	if (ZodiacPresentationData)
	{
		ZodiacPresentationData->OnDataTableLoaded.RemoveAll(this);
	}
	ReleasePresentationLevel();
	Super::NativeDestruct();
}

FReply UDBACharacterCreateFlowWidgetBase::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (IsBlockedCharacterCreateNavigationKey(InKeyEvent.GetKey()))
	{
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UDBACharacterCreateFlowWidgetBase::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (IsBlockedCharacterCreateNavigationKey(InKeyEvent.GetKey()))
	{
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UDBACharacterCreateFlowWidgetBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FVector2D ScreenPosition = InMouseEvent.GetScreenSpacePosition();
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton
		&& !IsPointerOverInteractiveControl(ScreenPosition))
	{
		BeginPreviewRotationDrag(ScreenPosition);
		TSharedRef<SWidget> SlateWidget = TakeWidget();
		return FReply::Handled()
			.SetUserFocus(SlateWidget)
			.CaptureMouse(SlateWidget);
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UDBACharacterCreateFlowWidgetBase::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsPreviewRotationDragging && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		EndPreviewRotationDrag();
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UDBACharacterCreateFlowWidgetBase::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsPreviewRotationDragging && InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		const FVector2D MouseDelta = InMouseEvent.GetCursorDelta();
		if (PresentationStage && FMath::Abs(MouseDelta.X) > KINDA_SMALL_NUMBER)
		{
			PresentationStage->AddPreviewYaw(MouseDelta.X * PreviewDragRotationDegreesPerPixel);
			LastPreviewDragScreenPosition = InMouseEvent.GetScreenSpacePosition();
		}
		else
		{
			UpdatePreviewRotationDrag(InMouseEvent.GetScreenSpacePosition());
		}
		return FReply::Handled();
	}

	if (bIsPreviewRotationDragging)
	{
		EndPreviewRotationDrag();
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UDBACharacterCreateFlowWidgetBase::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	const FVector2D ScreenPosition = InGestureEvent.GetScreenSpacePosition();
	if (!IsPointerOverInteractiveControl(ScreenPosition))
	{
		BeginPreviewRotationDrag(ScreenPosition);
		return FReply::Handled();
	}

	return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

FReply UDBACharacterCreateFlowWidgetBase::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (bIsPreviewRotationDragging)
	{
		UpdatePreviewRotationDrag(InGestureEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}

	return Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
}

FReply UDBACharacterCreateFlowWidgetBase::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (bIsPreviewRotationDragging)
	{
		EndPreviewRotationDrag();
		return FReply::Handled();
	}

	return Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
}

void UDBACharacterCreateFlowWidgetBase::SetCharacterName(const FString& Name)
{
	CharacterName = Name.TrimStartAndEnd();
	if (CharacterNameInput && CharacterNameInput->GetText().ToString() != CharacterName)
	{
		CharacterNameInput->SetText(FText::FromString(CharacterName));
	}
	else if (CharacterNameEditableText && CharacterNameEditableText->GetText().ToString() != CharacterName)
	{
		CharacterNameEditableText->SetText(FText::FromString(CharacterName));
	}
	Validate();
}

void UDBACharacterCreateFlowWidgetBase::SetZodiac(EDBAZodiac Zodiac)
{
	SelectedZodiac = Zodiac == EDBAZodiac::None ? EDBAZodiac::Rat : Zodiac;
	RefreshChoiceText();
	RefreshPresentedCharacter();
	Validate();
}

void UDBACharacterCreateFlowWidgetBase::SetElement(EDBAElement Element)
{
	SelectedElement = Element == EDBAElement::None ? EDBAElement::Water : Element;
	RefreshChoiceText();
	Validate();
}

void UDBACharacterCreateFlowWidgetBase::SetFiveCamp(EDBAFiveCamp FiveCamp)
{
	SelectedFiveCamp = FiveCamp;
	RefreshChoiceText();
	Validate();
}

void UDBACharacterCreateFlowWidgetBase::Submit()
{
	if (bIsSubmittingCreate)
	{
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 忽略重复的角色创建提交。"));
		return;
	}

	if (!Validate())
	{
		if (CharacterName.IsEmpty())
		{
			if (CharacterNameInput)
			{
				CharacterNameInput->SetKeyboardFocus();
			}
			else if (CharacterNameEditableText)
			{
				CharacterNameEditableText->SetKeyboardFocus();
			}
		}
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterCreateWidget] 角色创建校验未通过：名称=%s，生肖=%d，元素=%d。"),
			*CharacterName,
			static_cast<int32>(SelectedZodiac),
			static_cast<int32>(SelectedElement));
		return;
	}

	FDBACharacterCreateRequest Request;
	Request.CharacterName = CharacterName;
	Request.Zodiac = SelectedZodiac;
	Request.PrimaryElement = SelectedElement;
	Request.FiveCamp = SelectedFiveCamp;
	Request.DefaultZodiac = SelectedZodiac;
	Request.DefaultElement = SelectedElement;
	Request.DefaultFiveCamp = SelectedFiveCamp;

	if (UDBAFrontendFlowController* FlowController = GetFrontendFlowController())
	{
		bIsSubmittingCreate = true;
		if (CreateButton)
		{
			CreateButton->SetIsEnabled(false);
		}
		ShowValidationMessage(true, NSLOCTEXT("DBACharacterCreateWidget", "Creating", "正在创建角色..."));
		if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
			{
				UIManager->ShowLobbyLoadingScreen();
			}
		}
		FlowController->SubmitCharacterCreation(Request);
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 已提交角色创建：%s"), *CharacterName);
	}
	else
	{
		ShowValidationMessage(false, NSLOCTEXT("DBACharacterCreateWidget", "FlowUnavailable", "登录流程不可用。"));
	}
}

void UDBACharacterCreateFlowWidgetBase::BackToCharacterSelect()
{
	if (UDBAFrontendFlowController* FlowController = GetFrontendFlowController())
	{
		FlowController->BackToCharacterSelect();
	}
}

void UDBACharacterCreateFlowWidgetBase::ApplyCharacterFlowViewportPresentation()
{
	DBAUIFonts::ApplyFullscreenFlowViewportPresentation(this);
	HideCreateWorldStageBackgroundImages(WidgetTree);
	ApplyCreateMenuInputMode(this);
	InitializePresentationLevel();

	FVector2D LocalSize = GetCachedGeometry().GetLocalSize();
	FVector2D ViewportSize = FVector2D::ZeroVector;
	if (GetWorld() && GetWorld()->GetGameViewport())
	{
		GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);
	}
	if (LocalSize.IsNearlyZero() && !ViewportSize.IsNearlyZero())
	{
		LocalSize = ViewportSize;
	}
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 已应用全屏布局：控件几何=%.0fx%.0f，视口=%.0fx%.0f"), LocalSize.X, LocalSize.Y, ViewportSize.X, ViewportSize.Y);
}

void UDBACharacterCreateFlowWidgetBase::HandleCreateClicked()
{
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 已点击创建并进入按钮。"));
	Submit();
}

void UDBACharacterCreateFlowWidgetBase::HandleBackClicked()
{
	BackToCharacterSelect();
}

void UDBACharacterCreateFlowWidgetBase::HandleZodiacClicked()
{
	static const TArray<EDBAZodiac> Values = { EDBAZodiac::Rat, EDBAZodiac::Ox, EDBAZodiac::Tiger, EDBAZodiac::Rabbit, EDBAZodiac::Dragon, EDBAZodiac::Snake, EDBAZodiac::Horse, EDBAZodiac::Goat, EDBAZodiac::Monkey, EDBAZodiac::Rooster, EDBAZodiac::Dog, EDBAZodiac::Pig };
	SetZodiac(CycleEnumValue(SelectedZodiac, Values));
}

void UDBACharacterCreateFlowWidgetBase::HandleElementClicked()
{
	static const TArray<EDBAElement> Values = { EDBAElement::Water, EDBAElement::Fire, EDBAElement::Wood, EDBAElement::Gold, EDBAElement::Earth };
	SetElement(CycleEnumValue(SelectedElement, Values));
}

void UDBACharacterCreateFlowWidgetBase::HandleFiveCampClicked()
{
	static const TArray<EDBAFiveCamp> Values = { EDBAFiveCamp::None, EDBAFiveCamp::East, EDBAFiveCamp::West, EDBAFiveCamp::South, EDBAFiveCamp::North, EDBAFiveCamp::Center };
	SetFiveCamp(CycleEnumValue(SelectedFiveCamp, Values));
}

void UDBACharacterCreateFlowWidgetBase::HandleCharacterNameChanged(const FText& NewText)
{
	CharacterName = NewText.ToString().TrimStartAndEnd();
	Validate();
}

void UDBACharacterCreateFlowWidgetBase::HandleFlowError(const FString& ErrorMessage)
{
	bIsSubmittingCreate = false;
	if (CreateButton)
	{
		CreateButton->SetIsEnabled(true);
	}
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->HideLobbyLoadingScreen();
		}
	}
	ShowValidationMessage(false, FText::FromString(ErrorMessage));
}

void UDBACharacterCreateFlowWidgetBase::EnsureNativeFallbackLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget || (CharacterNameInput && CreateButton))
	{
		return;
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NativeCharacterCreateRoot"));
	WidgetTree->RootWidget = RootBox;

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NativeCreateTitle"));
	TitleText->SetText(NSLOCTEXT("DBACharacterCreateWidget", "Title", "创建角色"));
	RootBox->AddChildToVerticalBox(TitleText);

	ValidationText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ValidationText"));
	RootBox->AddChildToVerticalBox(ValidationText);

	CharacterNameInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("CharacterNameInput"));
	CharacterNameInput->SetHintText(NSLOCTEXT("DBACharacterCreateWidget", "NameHint", "请输入角色名称"));
	RootBox->AddChildToVerticalBox(CharacterNameInput);

	ZodiacButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ZodiacButton"));
	ZodiacText = MakeCreateButtonLabel(WidgetTree, FText::GetEmpty());
	ZodiacButton->AddChild(ZodiacText);
	RootBox->AddChildToVerticalBox(ZodiacButton);

	ElementButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ElementButton"));
	ElementText = MakeCreateButtonLabel(WidgetTree, FText::GetEmpty());
	ElementButton->AddChild(ElementText);
	RootBox->AddChildToVerticalBox(ElementButton);

	FiveCampButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("FiveCampButton"));
	FiveCampText = MakeCreateButtonLabel(WidgetTree, FText::GetEmpty());
	FiveCampButton->AddChild(FiveCampText);
	RootBox->AddChildToVerticalBox(FiveCampButton);

	CreateButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CreateButton"));
	CreateButton->AddChild(MakeCreateButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterCreateWidget", "CreateButton", "创建并进入")));
	RootBox->AddChildToVerticalBox(CreateButton);

	BackButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BackButton"));
	BackButton->AddChild(MakeCreateButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterCreateWidget", "BackButton", "返回角色选择")));
	RootBox->AddChildToVerticalBox(BackButton);

	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 已创建 C++ 原生兜底布局"));
}

void UDBACharacterCreateFlowWidgetBase::ResolveBoundWidgetsFromWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	auto FindNamedWidget = [this](const TArray<FName>& Names) -> UWidget*
	{
		for (const FName& Name : Names)
		{
			if (UWidget* Found = WidgetTree->FindWidget(Name))
			{
				return Found;
			}
		}
		return nullptr;
	};

	if (!CharacterNameInput && !CharacterNameEditableText)
	{
		CharacterNameInput = Cast<UEditableTextBox>(FindNamedWidget({ TEXT("CharacterNameInput"), TEXT("NameInput"), TEXT("RoleNameInput"), TEXT("ZodiacRow") }));
		if (!CharacterNameInput)
		{
			CharacterNameEditableText = Cast<UEditableText>(FindNamedWidget({ TEXT("CharacterNameInput"), TEXT("NameInput"), TEXT("RoleNameInput"), TEXT("NameText"), TEXT("NameTextInput") }));
		}
	}
	if (!CharacterNameInput && !CharacterNameEditableText)
	{
		WidgetTree->ForEachWidgetAndDescendants(
			[this](UWidget* Widget)
			{
				if (CharacterNameInput || CharacterNameEditableText || !Widget)
				{
					return;
				}

				const FString WidgetName = Widget->GetName();
				if (!WidgetName.Contains(TEXT("Name"), ESearchCase::IgnoreCase)
					&& !WidgetName.Contains(TEXT("Role"), ESearchCase::IgnoreCase)
					&& !WidgetName.Contains(TEXT("Character"), ESearchCase::IgnoreCase))
				{
					return;
				}

				CharacterNameInput = Cast<UEditableTextBox>(Widget);
				if (!CharacterNameInput)
				{
					CharacterNameEditableText = Cast<UEditableText>(Widget);
				}
			});
	}
	if (!ValidationText)
	{
		ValidationText = Cast<UTextBlock>(FindNamedWidget({ TEXT("ValidationText"), TEXT("StatusText"), TEXT("HintText") }));
	}
	if (!ZodiacButton)
	{
		ZodiacButton = Cast<UButton>(FindNamedWidget({ TEXT("ZodiacButton"), TEXT("RaceButton") }));
	}
	if (!ElementButton)
	{
		ElementButton = Cast<UButton>(FindNamedWidget({ TEXT("ElementButton"), TEXT("ElementSelectButton") }));
	}
	if (!FiveCampButton)
	{
		FiveCampButton = Cast<UButton>(FindNamedWidget({ TEXT("FiveCampButton"), TEXT("CampButton") }));
	}
	if (!CreateButton)
	{
		CreateButton = Cast<UButton>(FindNamedWidget({ TEXT("CreateButton"), TEXT("CreateCharacterButton"), TEXT("ConfirmCreateButton") }));
	}
	if (!BackButton)
	{
		BackButton = Cast<UButton>(FindNamedWidget({ TEXT("BackButton"), TEXT("BackToSelectButton") }));
	}
	if (!ZodiacText)
	{
		ZodiacText = Cast<UTextBlock>(FindNamedWidget({ TEXT("ZodiacText"), TEXT("RaceText") }));
	}
	if (!ZodiacText)
	{
		ZodiacText = GetCreateButtonLabel(ZodiacButton);
	}
	if (!ElementText)
	{
		ElementText = Cast<UTextBlock>(FindNamedWidget({ TEXT("ElementText") }));
	}
	if (!ElementText)
	{
		ElementText = GetCreateButtonLabel(ElementButton);
	}
	if (!FiveCampText)
	{
		FiveCampText = Cast<UTextBlock>(FindNamedWidget({ TEXT("FiveCampText"), TEXT("CampText") }));
	}
	if (!FiveCampText)
	{
		FiveCampText = GetCreateButtonLabel(FiveCampButton);
	}
	if (!CharacterNameInput && !CharacterNameEditableText)
	{
		UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
		if (!RootCanvas)
		{
			WidgetTree->ForEachWidgetAndDescendants(
				[&RootCanvas](UWidget* Widget)
				{
					if (!RootCanvas)
					{
						RootCanvas = Cast<UCanvasPanel>(Widget);
					}
				});
		}

		if (RootCanvas)
		{
			CharacterNameInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("CharacterNameInput_RuntimeFallback"));
			CharacterNameInput->SetHintText(NSLOCTEXT("DBACharacterCreateWidget", "NameHint", "请输入角色名称"));
			RootCanvas->AddChild(CharacterNameInput);
			UE_LOG(LogDBAUI, Warning, TEXT("[CharacterCreateWidget] 蓝图缺少角色名输入框，已注入 C++ 运行时输入控件。"));
		}
		else
		{
			UE_LOG(LogDBAUI, Error, TEXT("[CharacterCreateWidget] 蓝图缺少角色名输入框且未找到 CanvasPanel，无法注入输入控件。"));
		}
	}

	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 控件绑定结果：名称输入框=%s，名称文本框=%s，创建按钮=%s，生肖按钮=%s，元素按钮=%s。"),
		CharacterNameInput ? *CharacterNameInput->GetName() : TEXT("NULL"),
		CharacterNameEditableText ? *CharacterNameEditableText->GetName() : TEXT("NULL"),
		CreateButton ? *CreateButton->GetName() : TEXT("NULL"),
		ZodiacButton ? *ZodiacButton->GetName() : TEXT("NULL"),
		ElementButton ? *ElementButton->GetName() : TEXT("NULL"));
}

void UDBACharacterCreateFlowWidgetBase::ApplyBlueprintLayoutOverrides()
{
	ApplyWidgetSlot(
		CharacterNameInput,
		FAnchors(0.78f, 0.23f, 0.96f, 0.29f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));
	ApplyWidgetSlot(
		CharacterNameEditableText,
		FAnchors(0.78f, 0.23f, 0.96f, 0.29f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplyWidgetSlot(
		ZodiacButton,
		FAnchors(0.78f, 0.33f, 0.96f, 0.39f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplyWidgetSlot(
		ElementButton,
		FAnchors(0.78f, 0.43f, 0.96f, 0.49f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplyWidgetSlot(
		FiveCampButton,
		FAnchors(0.78f, 0.53f, 0.96f, 0.59f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplyWidgetSlot(
		CreateButton,
		FAnchors(0.78f, 0.70f, 0.96f, 0.76f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplyWidgetSlot(
		BackButton,
		FAnchors(0.78f, 0.79f, 0.96f, 0.85f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));
}

void UDBACharacterCreateFlowWidgetBase::ApplyLocalizedText()
{
	const FText TitleText = NSLOCTEXT("DBACharacterCreateWidget", "Title", "创建角色");
	const FText NameHintText = NSLOCTEXT("DBACharacterCreateWidget", "NameHint", "请输入角色名称");
	const FText CreateText = NSLOCTEXT("DBACharacterCreateWidget", "CreateButton", "创建并进入");
	const FText BackText = NSLOCTEXT("DBACharacterCreateWidget", "BackButton", "返回角色选择");

	if (CharacterNameInput)
	{
		CharacterNameInput->SetHintText(NameHintText);
	}
	if (CharacterNameEditableText)
	{
		CharacterNameEditableText->SetHintText(NameHintText);
	}

	SetCreateTextBlockByNames(WidgetTree, { TEXT("NativeCreateTitle"), TEXT("TitleText"), TEXT("CharacterCreateTitle"), TEXT("CreateTitle") }, TitleText);
	SetCreateTextBlockByNames(WidgetTree, { TEXT("CreateButtonText"), TEXT("ConfirmCreateButtonText") }, CreateText);
	SetCreateTextBlockByNames(WidgetTree, { TEXT("BackButtonText"), TEXT("BackToSelectText"), TEXT("BackToSelectButtonText") }, BackText);

	SetCreateButtonLabel(CreateButton, CreateText);
	SetCreateButtonLabel(BackButton, BackText);

	ReplaceCreateTextBlockValue(WidgetTree, TEXT("Create Character"), TitleText);
	ReplaceCreateTextBlockValue(WidgetTree, TEXT("Create and Enter"), CreateText);
	ReplaceCreateTextBlockValue(WidgetTree, TEXT("Back to Character Select"), BackText);
	ReplaceCreateTextBlockValue(WidgetTree, TEXT("Text Block"), FText::GetEmpty());
}

void UDBACharacterCreateFlowWidgetBase::BindControls()
{
	if (CharacterNameInput)
	{
		CharacterNameInput->OnTextChanged.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleCharacterNameChanged);
		CharacterNameInput->OnTextChanged.AddDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleCharacterNameChanged);
	}
	if (CharacterNameEditableText)
	{
		CharacterNameEditableText->OnTextChanged.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleCharacterNameChanged);
		CharacterNameEditableText->OnTextChanged.AddDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleCharacterNameChanged);
	}
	if (CreateButton)
	{
		CreateButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleCreateClicked);
		CreateButton->OnClicked.AddDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleCreateClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleBackClicked);
		BackButton->OnClicked.AddDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleBackClicked);
	}
	if (ZodiacButton)
	{
		ZodiacButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleZodiacClicked);
		ZodiacButton->OnClicked.AddDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleZodiacClicked);
	}
	if (ElementButton)
	{
		ElementButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleElementClicked);
		ElementButton->OnClicked.AddDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleElementClicked);
	}
	if (FiveCampButton)
	{
		FiveCampButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleFiveCampClicked);
		FiveCampButton->OnClicked.AddDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleFiveCampClicked);
	}
}

void UDBACharacterCreateFlowWidgetBase::UnbindControls()
{
	if (CharacterNameInput)
	{
		CharacterNameInput->OnTextChanged.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleCharacterNameChanged);
	}
	if (CharacterNameEditableText)
	{
		CharacterNameEditableText->OnTextChanged.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleCharacterNameChanged);
	}
	if (CreateButton)
	{
		CreateButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleCreateClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleBackClicked);
	}
	if (ZodiacButton)
	{
		ZodiacButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleZodiacClicked);
	}
	if (ElementButton)
	{
		ElementButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleElementClicked);
	}
	if (FiveCampButton)
	{
		FiveCampButton->OnClicked.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleFiveCampClicked);
	}
}

bool UDBACharacterCreateFlowWidgetBase::Validate()
{
	FText ValidationMessage;
	const bool bNameValid = ValidateCharacterName(ValidationMessage);
	bIsCreateValid = bNameValid && SelectedZodiac != EDBAZodiac::None && SelectedElement != EDBAElement::None;

	if (!bNameValid)
	{
		ShowValidationMessage(false, ValidationMessage);
		return false;
	}

	if (SelectedZodiac == EDBAZodiac::None || SelectedElement == EDBAElement::None)
	{
		ShowValidationMessage(false, NSLOCTEXT("DBACharacterCreateWidget", "MissingSelection", "请选择生肖和元素。"));
		return false;
	}

	ShowValidationMessage(true, NSLOCTEXT("DBACharacterCreateWidget", "Ready", "已准备好。"));
	return bIsCreateValid;
}

bool UDBACharacterCreateFlowWidgetBase::ValidateCharacterName(FText& OutMessage) const
{
	if (CharacterName.IsEmpty())
	{
		OutMessage = NSLOCTEXT("DBACharacterCreateWidget", "MissingName", "请输入角色名称。");
		return false;
	}

	const int32 NameLen = CharacterName.Len();
	if (NameLen < 2 || NameLen > 16)
	{
		OutMessage = NSLOCTEXT("DBACharacterCreateWidget", "NameLen", "名称长度需为 2-16 个字符。");
		return false;
	}

	bool bAllDigits = true;
	for (const TCHAR Ch : CharacterName)
	{
		const bool bIsAsciiWord = FChar::IsAlnum(Ch) || Ch == TEXT('_');
		const bool bIsCJK = Ch >= 0x4E00 && Ch <= 0x9FFF;
		if (!bIsAsciiWord && !bIsCJK)
		{
			OutMessage = NSLOCTEXT("DBACharacterCreateWidget", "NameChars", "名称仅支持中文、字母、数字和下划线。");
			return false;
		}
		if (!FChar::IsDigit(Ch))
		{
			bAllDigits = false;
		}
	}

	if (bAllDigits)
	{
		OutMessage = NSLOCTEXT("DBACharacterCreateWidget", "AllDigits", "名称不能全部为数字。");
		return false;
	}

	OutMessage = FText::GetEmpty();
	return true;
}

void UDBACharacterCreateFlowWidgetBase::RefreshChoiceText()
{
	if (ZodiacText)
	{
		FText PresentationText;
		if (ZodiacPresentationData && ZodiacPresentationData->GetCharacterSelectionSummaryText(SelectedZodiac, SelectedElement, PresentationText))
		{
			ZodiacText->SetText(PresentationText);
		}
		else
		{
			ZodiacText->SetText(FText::Format(NSLOCTEXT("DBACharacterCreateWidget", "ZodiacFormat", "生肖：{0}"), EnumValueText(SelectedZodiac)));
		}
	}
	if (ElementText)
	{
		ElementText->SetText(FText::Format(NSLOCTEXT("DBACharacterCreateWidget", "ElementFormat", "元素：{0}"), EnumValueText(SelectedElement)));
	}
	if (FiveCampText)
	{
		FiveCampText->SetText(FText::Format(NSLOCTEXT("DBACharacterCreateWidget", "CampFormat", "阵营：{0}"), EnumValueText(SelectedFiveCamp)));
	}
}

void UDBACharacterCreateFlowWidgetBase::InitializeZodiacPresentationData()
{
	const UDBAUIDeveloperSettings* UISettings = GetDefault<UDBAUIDeveloperSettings>();
	if (!UISettings || UISettings->ZodiacCharacterSelectionData.IsNull())
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[角色创建界面] 未配置十二生肖角色数据资产。"));
		return;
	}

	TWeakObjectPtr<UDBACharacterCreateFlowWidgetBase> WeakThis(this);
	DBAAsyncAssetLoader::RequestAsyncAsset<UDBAZodiacHeroDataAsset>(this, UISettings->ZodiacCharacterSelectionData, [WeakThis](UDBAZodiacHeroDataAsset* LoadedAsset)
	{
		if (!WeakThis.IsValid() || !LoadedAsset)
		{
			return;
		}

		WeakThis->ZodiacPresentationData = LoadedAsset;
		LoadedAsset->OnDataTableLoaded.AddUObject(WeakThis.Get(), &UDBACharacterCreateFlowWidgetBase::HandleZodiacPresentationDataTableLoaded);
		LoadedAsset->PreloadAllDataTablesAsync();
		WeakThis->RefreshChoiceText();
	});
}

void UDBACharacterCreateFlowWidgetBase::HandleZodiacPresentationDataTableLoaded(UDataTable* LoadedTable, const FSoftObjectPath& AssetPath)
{
	UE_LOG(LogDBAUI, Verbose, TEXT("[角色创建界面] 十二生肖展示数据已加载：%s"), *AssetPath.ToString());
	RefreshChoiceText();
}

void UDBACharacterCreateFlowWidgetBase::ShowValidationMessage(bool bValid, const FText& Message)
{
	if (ValidationText)
	{
		ValidationText->SetText(Message);
		ValidationText->SetVisibility(Message.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	BP_OnValidationChanged(bValid, Message);
}

UDBAFrontendFlowSubsystem* UDBACharacterCreateFlowWidgetBase::GetLoginFlow() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr;
}

UDBAFrontendFlowController* UDBACharacterCreateFlowWidgetBase::GetFrontendFlowController() const
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

void UDBACharacterCreateFlowWidgetBase::InitializePresentationLevel()
{
	if (InjectedBackgroundImage)
	{
		InjectedBackgroundImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
		InjectedBackgroundImage->SetRenderOpacity(0.0f);
		InjectedBackgroundImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	HideCreateWorldStageBackgroundImages(WidgetTree);

	if (UWorld* World = GetWorld())
	{
		if (UDBAFrontendEnvironmentSubsystem* FrontendEnvironment = World->GetSubsystem<UDBAFrontendEnvironmentSubsystem>())
		{
			FrontendEnvironment->EnableCharacterPresentationRendering();
		}
	}

	BindPlacedPresentationStage();

	if (UWorld* World = GetWorld())
	{
		DeferredPresentationActivateRetryCount = 0;
		World->GetTimerManager().ClearTimer(DeferredPresentationActivateTimerHandle);
		World->GetTimerManager().SetTimer(
			DeferredPresentationActivateTimerHandle,
			this,
			&UDBACharacterCreateFlowWidgetBase::HandleDeferredPresentationStageBinding,
			0.05f,
			true);
	}
}

void UDBACharacterCreateFlowWidgetBase::BindPlacedPresentationStage()
{
	SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	SetRenderOpacity(1.0f);

	if (!PresentationStage)
	{
		PresentationStage = ADBACharacterPresentationActor::FindPlacedPresentationStage(GetWorld());
	}

	if (!PresentationStage)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[角色创建界面] 固定关卡中未放置角色展示 Actor，无法显示角色。"));
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC && GetWorld())
	{
		PC = GetWorld()->GetFirstPlayerController();
	}

	PresentationStage->SetActorHiddenInGame(false);
	PresentationStage->ActivatePresentationCamera(PC, 0.0f);
	PresentationStage->SetPreviewZodiac(SelectedZodiac);

	AActor* ViewTarget = PC ? PC->GetViewTarget() : nullptr;
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] 使用世界 3D 角色展示舞台：生肖=%d ViewTarget=%s 舞台=%s"),
		static_cast<int32>(SelectedZodiac),
		ViewTarget ? *ViewTarget->GetName() : TEXT("无"),
		*PresentationStage->GetName());
}

void UDBACharacterCreateFlowWidgetBase::HandleDeferredPresentationStageBinding()
{
	BindPlacedPresentationStage();

	++DeferredPresentationActivateRetryCount;
	if (DeferredPresentationActivateRetryCount >= 8)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DeferredPresentationActivateTimerHandle);
		}
		DeferredPresentationActivateRetryCount = 0;
	}
}

void UDBACharacterCreateFlowWidgetBase::ReleasePresentationLevel()
{
	// 固定关卡舞台由关卡生命周期管理，界面销毁时仅释放弱关联。
	PresentationStage = nullptr;
}

bool UDBACharacterCreateFlowWidgetBase::IsPointerOverInteractiveControl(const FVector2D& ScreenPosition) const
{
	const TArray<const UWidget*> InteractiveControls = {
		CharacterNameInput,
		CharacterNameEditableText,
		ZodiacButton,
		ElementButton,
		FiveCampButton,
		CreateButton,
		BackButton
	};

	for (const UWidget* Control : InteractiveControls)
	{
		const ESlateVisibility ControlVisibility = Control ? Control->GetVisibility() : ESlateVisibility::Collapsed;
		if (Control
			&& ControlVisibility != ESlateVisibility::Hidden
			&& ControlVisibility != ESlateVisibility::Collapsed
			&& Control->GetCachedGeometry().IsUnderLocation(ScreenPosition))
		{
			return true;
		}
	}

	return false;
}

void UDBACharacterCreateFlowWidgetBase::BeginPreviewRotationDrag(const FVector2D& ScreenPosition)
{
	if (!PresentationStage)
	{
		return;
	}

	bIsPreviewRotationDragging = true;
	LastPreviewDragScreenPosition = ScreenPosition;
}

void UDBACharacterCreateFlowWidgetBase::UpdatePreviewRotationDrag(const FVector2D& ScreenPosition)
{
	if (!bIsPreviewRotationDragging || !PresentationStage)
	{
		return;
	}

	const float DeltaX = ScreenPosition.X - LastPreviewDragScreenPosition.X;
	if (FMath::Abs(DeltaX) > KINDA_SMALL_NUMBER)
	{
		PresentationStage->AddPreviewYaw(DeltaX * PreviewDragRotationDegreesPerPixel);
	}

	LastPreviewDragScreenPosition = ScreenPosition;
}

void UDBACharacterCreateFlowWidgetBase::EndPreviewRotationDrag()
{
	bIsPreviewRotationDragging = false;
}

void UDBACharacterCreateFlowWidgetBase::RefreshPresentedCharacter()
{
	if (PresentationStage)
	{
		PresentationStage->SetPreviewZodiac(SelectedZodiac);
	}
}

void UDBACharacterCreateFlowWidgetBase::HandleBackgroundMusicFinished()
{
	StartBackgroundMusic();
}

void UDBACharacterCreateFlowWidgetBase::InitializeAudioAssets()
{
	const UDBAUIDeveloperSettings* UISettings = GetDefault<UDBAUIDeveloperSettings>();
	if (!UISettings)
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterCreateWidget] 未找到 UI 音频配置。"));
		return;
	}

	if (!ButtonClickSound)
	{
		DBAAsyncAssetLoader::RequestAsyncAsset<USoundBase>(this, UISettings->UIButtonClickSFX,
			[this](USoundBase* LoadedSound)
			{
				ButtonClickSound = LoadedSound;
			});
	}

	if (!BackgroundMusicSound)
	{
		DBAAsyncAssetLoader::RequestAsyncAsset<USoundBase>(this, UISettings->CharacterCreateBGM,
			[this](USoundBase* LoadedSound)
			{
				BackgroundMusicSound = LoadedSound;
			});
	}
}

void UDBACharacterCreateFlowWidgetBase::StartBackgroundMusic()
{
	if (BackgroundMusicComponent || !BackgroundMusicSound || !GetWorld())
	{
		return;
	}

	BackgroundMusicComponent = UGameplayStatics::SpawnSound2D(GetWorld(), BackgroundMusicSound, 0.4f, 1.0f, 0.0f, nullptr, false, false);
	if (BackgroundMusicComponent)
	{
		BackgroundMusicComponent->bIsUISound = true;
		BackgroundMusicComponent->SetVolumeMultiplier(0.75f);
		BackgroundMusicComponent->OnAudioFinished.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleBackgroundMusicFinished);
		BackgroundMusicComponent->OnAudioFinished.AddDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleBackgroundMusicFinished);
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterCreateWidget] 创建背景音乐组件失败。"));
	}
}

void UDBACharacterCreateFlowWidgetBase::StopBackgroundMusic()
{
	if (!BackgroundMusicComponent)
	{
		return;
	}

	BackgroundMusicComponent->OnAudioFinished.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleBackgroundMusicFinished);
	BackgroundMusicComponent->Stop();
	BackgroundMusicComponent = nullptr;
}

void UDBACharacterCreateFlowWidgetBase::PlayButtonClickSfx() const
{
	if (ButtonClickSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ButtonClickSound, 0.85f);
	}
}
