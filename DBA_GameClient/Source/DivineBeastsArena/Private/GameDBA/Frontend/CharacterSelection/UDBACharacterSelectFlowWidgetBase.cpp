// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Frontend/CharacterSelection/UDBACharacterSelectFlowWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/AudioComponent.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ContentWidget.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameDBA/Frontend/Character/DBACharacterRosterSubsystem.h"
#include "GameDBA/Frontend/DBAFrontendEnvironmentSubsystem.h"
#include "GameDBA/UI/Controllers/DBAGameUIManager.h"
#include "GameDBA/UI/Frontend/DBAFrontendFlowController.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Data/Assets/DBAZodiacHeroDataAsset.h"
#include "GameDBA/UI/DBAUIDeveloperSettings.h"
#include "GameDBA/UI/DBAUIFontUtils.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterPresentationActor.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewSubsystem.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewStage.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace
{
	UTextBlock* MakeSelectButtonLabel(UWidgetTree* WidgetTree, const FText& Label)
	{
		UTextBlock* TextBlock = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()) : nullptr;
		if (TextBlock)
		{
			TextBlock->SetText(Label);
		}
		return TextBlock;
	}

	UTextBlock* GetSelectButtonLabel(UButton* Button)
	{
		if (const UContentWidget* ContentHost = Cast<UContentWidget>(Button))
		{
			return Cast<UTextBlock>(ContentHost->GetContent());
		}
		return nullptr;
	}

	void SetSelectButtonLabel(UButton* Button, const FText& Label)
	{
		if (UTextBlock* TextBlock = GetSelectButtonLabel(Button))
		{
			TextBlock->SetText(Label);
		}
	}

	void SetSelectTextBlockByNames(UWidgetTree* WidgetTree, const TArray<FName>& Names, const FText& Text)
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

	void ReplaceSelectTextBlockValue(UWidgetTree* WidgetTree, const FString& EnglishText, const FText& ChineseText)
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

	bool IsCharacterSelectCycleKey(const FKey& Key)
	{
		return Key == EKeys::Left || Key == EKeys::Right || Key == EKeys::A || Key == EKeys::D;
	}

	bool IsSelectUiChromeWidgetName(const FString& Name)
	{
		const FString LowerName = Name.ToLower();
		return LowerName.Contains(TEXT("button"))
			|| LowerName.Contains(TEXT("title"))
			|| LowerName.Contains(TEXT("status"))
			|| LowerName.Contains(TEXT("hint"))
			|| LowerName.Contains(TEXT("list"))
			|| LowerName.Contains(TEXT("confirm"))
			|| LowerName.Contains(TEXT("create"))
			|| LowerName.Contains(TEXT("refresh"))
			|| LowerName.Contains(TEXT("prev"))
			|| LowerName.Contains(TEXT("next"))
			|| LowerName.Contains(TEXT("label"))
			|| LowerName.Contains(TEXT("text"));
	}

	bool IsSelectBackgroundImageName(const FString& Name)
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

	bool IsSelectFullBleedCanvasSlot(const UWidget* Widget)
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

	void ApplySelectWidgetSlot(UWidget* Widget, const FAnchors& Anchors, const FMargin& Offsets, const FVector2D& Alignment)
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

	void HideSelectWorldStageBackgroundImages(UWidgetTree* WidgetTree)
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
				if (IsSelectUiChromeWidgetName(WidgetName))
				{
					return;
				}

				const bool bNamedBackground = IsSelectBackgroundImageName(WidgetName);
				const bool bFullBleed = IsSelectFullBleedCanvasSlot(Widget);
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

		UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] 已透明化遮挡世界舞台的背景控件数量=%d"), ClearedCount);
	}

	void ApplySelectMenuInputMode(UUserWidget* Widget)
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

UDBACharacterSelectFlowWidgetBase::UDBACharacterSelectFlowWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	// 选角界面需要透视世界 3D 舞台（角色/灯光/地板），禁止基类注入不透明背景。
	bAutoInjectBackground = false;
	BackgroundOpacity = 0.0f;
}

void UDBACharacterSelectFlowWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBACharacterSelectFlowWidgetBase::NativeConstruct()
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
	ApplySelectMenuInputMode(this);
	InitializePresentationLevel();
	InitializeZodiacPresentationData();
	bIsSubmittingSelection = false;
	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(true);
	}

	if (UDBAFrontendFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->OnCharactersLoaded.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleCharactersLoaded);
		LoginFlow->OnCharactersLoaded.AddDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleCharactersLoaded);
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleFlowError);
		LoginFlow->OnFlowError.AddDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleFlowError);
		UpdateCharacters(LoginFlow->GetCachedCharacters());
	}
}

void UDBACharacterSelectFlowWidgetBase::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeferredPresentationActivateTimerHandle);
	}
	DeferredPresentationActivateRetryCount = 0;

	if (UDBAFrontendFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->OnCharactersLoaded.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleCharactersLoaded);
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleFlowError);
	}

	UnbindControls();
	if (ZodiacPresentationData)
	{
		ZodiacPresentationData->OnDataTableLoaded.RemoveAll(this);
	}
	ReleasePresentationLevel();
	Super::NativeDestruct();
}

FReply UDBACharacterSelectFlowWidgetBase::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (IsCharacterSelectCycleKey(Key))
	{
		if (Key == EKeys::Left || Key == EKeys::A)
		{
			SelectPreviousCharacter();
		}
		else
		{
			SelectNextCharacter();
		}
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UDBACharacterSelectFlowWidgetBase::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (IsCharacterSelectCycleKey(Key))
	{
		if (Key == EKeys::Left || Key == EKeys::A)
		{
			SelectPreviousCharacter();
		}
		else
		{
			SelectNextCharacter();
		}
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UDBACharacterSelectFlowWidgetBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton
		&& !IsPointerOverInteractiveControl(InMouseEvent.GetScreenSpacePosition()))
	{
		BeginPreviewRotationDrag(InMouseEvent.GetScreenSpacePosition());
		TSharedRef<SWidget> SlateWidget = TakeWidget();
		return FReply::Handled()
			.SetUserFocus(SlateWidget)
			.CaptureMouse(SlateWidget);
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UDBACharacterSelectFlowWidgetBase::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsPreviewRotationDragging && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		EndPreviewRotationDrag();
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UDBACharacterSelectFlowWidgetBase::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsPreviewRotationDragging && InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		const FVector2D MouseDelta = InMouseEvent.GetCursorDelta();
		if (FMath::Abs(MouseDelta.X) > KINDA_SMALL_NUMBER)
		{
			if (UDBACharacterPreviewSubsystem* PreviewSubsystem = GetGameInstance()->GetSubsystem<UDBACharacterPreviewSubsystem>())
			{
				PreviewSubsystem->Rotate(MouseDelta.X * PreviewDragRotationDegreesPerPixel);
			}
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

FReply UDBACharacterSelectFlowWidgetBase::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (!IsPointerOverInteractiveControl(InGestureEvent.GetScreenSpacePosition()))
	{
		BeginPreviewRotationDrag(InGestureEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}

	return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

FReply UDBACharacterSelectFlowWidgetBase::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (bIsPreviewRotationDragging)
	{
		UpdatePreviewRotationDrag(InGestureEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}

	return Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
}

FReply UDBACharacterSelectFlowWidgetBase::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (bIsPreviewRotationDragging)
	{
		EndPreviewRotationDrag();
		return FReply::Handled();
	}

	return Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
}

void UDBACharacterSelectFlowWidgetBase::UpdateCharacters(const TArray<FDBACharacterSummary>& Characters)
{
	CurrentCharacters = Characters;

	const bool bSelectionStillValid = SelectedCharacterId.IsValid() && CurrentCharacters.ContainsByPredicate(
		[this](const FDBACharacterSummary& Character)
		{
			return Character.CharacterId == SelectedCharacterId;
		});
	if (!bSelectionStillValid)
	{
		SelectedCharacterId = CurrentCharacters.Num() > 0 ? CurrentCharacters[0].CharacterId : FDBACharacterId();
	}

	RefreshCharacterText();
	UpdatePresentedCharacterById(SelectedCharacterId);
	BP_OnCharactersUpdated(Characters);
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] 角色列表已更新：%d"), Characters.Num());
}

void UDBACharacterSelectFlowWidgetBase::SelectCharacter(const FDBACharacterId& CharacterId)
{
	SelectedCharacterId = CharacterId;
	BP_OnCharacterSelected(CharacterId);
	RefreshCharacterText();
	UpdatePresentedCharacterById(CharacterId);
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] 已选择角色：%s"), *CharacterId.ToString());
}

void UDBACharacterSelectFlowWidgetBase::ConfirmSelectedCharacter()
{
	if (bIsSubmittingSelection)
	{
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] 忽略重复的角色选择提交。"));
		return;
	}

	if (!SelectedCharacterId.IsValid() && CurrentCharacters.Num() > 0)
	{
		SelectedCharacterId = CurrentCharacters[0].CharacterId;
	}

	if (!SelectedCharacterId.IsValid() && CurrentCharacters.Num() <= 0)
	{
		if (UDBAFrontendFlowController* FlowController = GetFrontendFlowController())
		{
			SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "NoCharactersEnterCreate", "\u6682\u65e0\u89d2\u8272\uff0c\u6b63\u5728\u8fdb\u5165\u521b\u5efa\u89d2\u8272\u3002"));
			FlowController->EnterCharacterCreate();
			return;
		}
	}

	if (!SelectedCharacterId.IsValid())
	{
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "NoSelection", "尚未选择角色。"));
		return;
	}

	if (UDBAFrontendFlowController* FlowController = GetFrontendFlowController())
	{
		bIsSubmittingSelection = true;
		if (ConfirmButton)
		{
			ConfirmButton->SetIsEnabled(false);
		}
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "EnteringLobby", "正在进入大厅..."));
		FlowController->SubmitCharacterSelection(SelectedCharacterId);
	}
	else
	{
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "FlowUnavailable", "登录流程不可用。"));
	}
}

void UDBACharacterSelectFlowWidgetBase::EnterCharacterCreate()
{
	if (UDBAFrontendFlowController* FlowController = GetFrontendFlowController())
	{
		FlowController->EnterCharacterCreate();
	}
}

void UDBACharacterSelectFlowWidgetBase::RefreshCharacterList()
{
	if (UDBAFrontendFlowController* FlowController = GetFrontendFlowController())
	{
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "Refreshing", "正在刷新角色列表..."));
		FlowController->RefreshCharacterList();
	}
}

void UDBACharacterSelectFlowWidgetBase::SelectPreviousCharacter()
{
	if (CurrentCharacters.Num() <= 0)
	{
		return;
	}

	int32 CurrentIndex = CurrentCharacters.IndexOfByPredicate(
		[this](const FDBACharacterSummary& Character)
		{
			return Character.CharacterId == SelectedCharacterId;
		});
	if (CurrentIndex == INDEX_NONE)
	{
		CurrentIndex = 0;
	}
	else
	{
		CurrentIndex = (CurrentIndex - 1 + CurrentCharacters.Num()) % CurrentCharacters.Num();
	}

	SelectCharacter(CurrentCharacters[CurrentIndex].CharacterId);
}

void UDBACharacterSelectFlowWidgetBase::SelectNextCharacter()
{
	if (CurrentCharacters.Num() <= 0)
	{
		return;
	}

	int32 CurrentIndex = CurrentCharacters.IndexOfByPredicate(
		[this](const FDBACharacterSummary& Character)
		{
			return Character.CharacterId == SelectedCharacterId;
		});
	if (CurrentIndex == INDEX_NONE)
	{
		CurrentIndex = 0;
	}
	else
	{
		CurrentIndex = (CurrentIndex + 1) % CurrentCharacters.Num();
	}

	SelectCharacter(CurrentCharacters[CurrentIndex].CharacterId);
}

void UDBACharacterSelectFlowWidgetBase::ApplyCharacterFlowViewportPresentation()
{
	DBAUIFonts::ApplyFullscreenFlowViewportPresentation(this);
	HideSelectWorldStageBackgroundImages(WidgetTree);
	ApplySelectMenuInputMode(this);
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
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] 已应用全屏布局：控件几何=%.0fx%.0f，视口=%.0fx%.0f"), LocalSize.X, LocalSize.Y, ViewportSize.X, ViewportSize.Y);
}

void UDBACharacterSelectFlowWidgetBase::HandleConfirmClicked()
{
	ConfirmSelectedCharacter();
}

void UDBACharacterSelectFlowWidgetBase::HandleCreateClicked()
{
	EnterCharacterCreate();
}

void UDBACharacterSelectFlowWidgetBase::HandleRefreshClicked()
{
	RefreshCharacterList();
}

void UDBACharacterSelectFlowWidgetBase::HandlePrevClicked()
{
	SelectPreviousCharacter();
}

void UDBACharacterSelectFlowWidgetBase::HandleNextClicked()
{
	SelectNextCharacter();
}

void UDBACharacterSelectFlowWidgetBase::HandleCharactersLoaded(const TArray<FDBACharacterSummary>& Characters)
{
	UpdateCharacters(Characters);
}

void UDBACharacterSelectFlowWidgetBase::HandleFlowError(const FString& ErrorMessage)
{
	bIsSubmittingSelection = false;
	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(true);
	}

	SetStatus(FText::FromString(ErrorMessage));
}

void UDBACharacterSelectFlowWidgetBase::EnsureNativeFallbackLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget || (CharacterListText && ConfirmButton))
	{
		return;
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NativeCharacterSelectRoot"));
	WidgetTree->RootWidget = RootBox;

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NativeSelectTitle"));
	TitleText->SetText(NSLOCTEXT("DBACharacterSelectWidget", "Title", "选择角色"));
	RootBox->AddChildToVerticalBox(TitleText);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	RootBox->AddChildToVerticalBox(StatusText);

	CharacterListText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CharacterListText"));
	RootBox->AddChildToVerticalBox(CharacterListText);

	ConfirmButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ConfirmButton"));
	ConfirmButton->AddChild(MakeSelectButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterSelectWidget", "ConfirmButton", "进入大厅")));
	RootBox->AddChildToVerticalBox(ConfirmButton);

	CreateButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CreateButton"));
	CreateButton->AddChild(MakeSelectButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterSelectWidget", "CreateButton", "创建角色")));
	RootBox->AddChildToVerticalBox(CreateButton);

	RefreshButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RefreshButton"));
	RefreshButton->AddChild(MakeSelectButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterSelectWidget", "RefreshButton", "刷新列表")));
	RootBox->AddChildToVerticalBox(RefreshButton);

	PrevCharacterButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PrevCharacterButton"));
	PrevCharacterButton->AddChild(MakeSelectButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterSelectWidget", "PrevButton", "上一个")));
	RootBox->AddChildToVerticalBox(PrevCharacterButton);

	NextCharacterButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("NextCharacterButton"));
	NextCharacterButton->AddChild(MakeSelectButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterSelectWidget", "NextButton", "下一个")));
	RootBox->AddChildToVerticalBox(NextCharacterButton);

	UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] 已创建 C++ 原生兜底布局"));
}

void UDBACharacterSelectFlowWidgetBase::ResolveBoundWidgetsFromWidgetTree()
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

	if (!CharacterListText)
	{
		CharacterListText = Cast<UTextBlock>(FindNamedWidget({ TEXT("CharacterListText"), TEXT("CharacterList"), TEXT("CharacterEntriesText") }));
	}
	if (!StatusText)
	{
		StatusText = Cast<UTextBlock>(FindNamedWidget({ TEXT("StatusText"), TEXT("HintText"), TEXT("FlowStatusText") }));
	}
	if (!ConfirmButton)
	{
		ConfirmButton = Cast<UButton>(FindNamedWidget({ TEXT("ConfirmButton"), TEXT("EnterLobbyButton"), TEXT("EnterButton") }));
	}
	if (!CreateButton)
	{
		CreateButton = Cast<UButton>(FindNamedWidget({ TEXT("CreateButton"), TEXT("CreateCharacterButton") }));
	}
	if (!RefreshButton)
	{
		RefreshButton = Cast<UButton>(FindNamedWidget({ TEXT("RefreshButton"), TEXT("ReloadButton"), TEXT("RefreshListButton") }));
	}
	if (!PrevCharacterButton)
	{
		PrevCharacterButton = Cast<UButton>(FindNamedWidget({ TEXT("PrevCharacterButton"), TEXT("PreviousButton"), TEXT("PrevButton"), TEXT("LeftCharacterButton") }));
	}
	if (!NextCharacterButton)
	{
		NextCharacterButton = Cast<UButton>(FindNamedWidget({ TEXT("NextCharacterButton"), TEXT("NextButton"), TEXT("RightCharacterButton") }));
	}
}

void UDBACharacterSelectFlowWidgetBase::ApplyBlueprintLayoutOverrides()
{
	ApplySelectWidgetSlot(
		CharacterListText,
		FAnchors(0.03f, 0.17f, 0.21f, 0.74f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplySelectWidgetSlot(
		RefreshButton,
		FAnchors(0.03f, 0.78f, 0.21f, 0.84f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplySelectWidgetSlot(
		PrevCharacterButton,
		FAnchors(0.03f, 0.86f, 0.11f, 0.92f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplySelectWidgetSlot(
		NextCharacterButton,
		FAnchors(0.13f, 0.86f, 0.21f, 0.92f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplySelectWidgetSlot(
		ConfirmButton,
		FAnchors(0.80f, 0.73f, 0.96f, 0.79f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplySelectWidgetSlot(
		CreateButton,
		FAnchors(0.80f, 0.82f, 0.96f, 0.88f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplySelectWidgetSlot(
		StatusText,
		FAnchors(0.79f, 0.90f, 0.97f, 0.96f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));
}

void UDBACharacterSelectFlowWidgetBase::ApplyLocalizedText()
{
	const FText TitleText = NSLOCTEXT("DBACharacterSelectWidget", "Title", "选择角色");
	const FText ConfirmText = NSLOCTEXT("DBACharacterSelectWidget", "ConfirmButton", "进入大厅");
	const FText CreateText = NSLOCTEXT("DBACharacterSelectWidget", "CreateButton", "创建角色");
	const FText RefreshText = NSLOCTEXT("DBACharacterSelectWidget", "RefreshButton", "刷新列表");
	const FText PrevText = NSLOCTEXT("DBACharacterSelectWidget", "PrevButton", "上一个");
	const FText NextText = NSLOCTEXT("DBACharacterSelectWidget", "NextButton", "下一个");

	SetSelectTextBlockByNames(WidgetTree, { TEXT("NativeSelectTitle"), TEXT("TitleText"), TEXT("CharacterSelectTitle"), TEXT("SelectTitle") }, TitleText);
	SetSelectTextBlockByNames(WidgetTree, { TEXT("ConfirmButtonText"), TEXT("EnterLobbyText"), TEXT("EnterText") }, ConfirmText);
	SetSelectTextBlockByNames(WidgetTree, { TEXT("CreateButtonText"), TEXT("CreateCharacterText") }, CreateText);
	SetSelectTextBlockByNames(WidgetTree, { TEXT("RefreshButtonText"), TEXT("RefreshListText"), TEXT("ReloadButtonText") }, RefreshText);
	SetSelectTextBlockByNames(WidgetTree, { TEXT("PrevButtonText"), TEXT("PreviousButtonText") }, PrevText);
	SetSelectTextBlockByNames(WidgetTree, { TEXT("NextButtonText") }, NextText);

	SetSelectButtonLabel(ConfirmButton, ConfirmText);
	SetSelectButtonLabel(CreateButton, CreateText);
	SetSelectButtonLabel(RefreshButton, RefreshText);
	SetSelectButtonLabel(PrevCharacterButton, PrevText);
	SetSelectButtonLabel(NextCharacterButton, NextText);

	ReplaceSelectTextBlockValue(WidgetTree, TEXT("Select Character"), TitleText);
	ReplaceSelectTextBlockValue(WidgetTree, TEXT("Enter Lobby"), ConfirmText);
	ReplaceSelectTextBlockValue(WidgetTree, TEXT("Create Character"), CreateText);
	ReplaceSelectTextBlockValue(WidgetTree, TEXT("Refresh"), RefreshText);
	ReplaceSelectTextBlockValue(WidgetTree, TEXT("Previous"), PrevText);
	ReplaceSelectTextBlockValue(WidgetTree, TEXT("Next"), NextText);
}

void UDBACharacterSelectFlowWidgetBase::BindControls()
{
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleConfirmClicked);
		ConfirmButton->OnClicked.AddDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleConfirmClicked);
	}
	if (CreateButton)
	{
		CreateButton->OnClicked.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleCreateClicked);
		CreateButton->OnClicked.AddDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleCreateClicked);
	}
	if (RefreshButton)
	{
		RefreshButton->OnClicked.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleRefreshClicked);
		RefreshButton->OnClicked.AddDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleRefreshClicked);
	}
	if (PrevCharacterButton)
	{
		PrevCharacterButton->OnClicked.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandlePrevClicked);
		PrevCharacterButton->OnClicked.AddDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandlePrevClicked);
	}
	if (NextCharacterButton)
	{
		NextCharacterButton->OnClicked.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleNextClicked);
		NextCharacterButton->OnClicked.AddDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleNextClicked);
	}
}

void UDBACharacterSelectFlowWidgetBase::UnbindControls()
{
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleConfirmClicked);
	}
	if (CreateButton)
	{
		CreateButton->OnClicked.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleCreateClicked);
	}
	if (RefreshButton)
	{
		RefreshButton->OnClicked.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleRefreshClicked);
	}
	if (PrevCharacterButton)
	{
		PrevCharacterButton->OnClicked.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandlePrevClicked);
	}
	if (NextCharacterButton)
	{
		NextCharacterButton->OnClicked.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleNextClicked);
	}
}

void UDBACharacterSelectFlowWidgetBase::RefreshCharacterText()
{
	if (!CharacterListText)
	{
		return;
	}

	if (CurrentCharacters.Num() == 0)
	{
		CharacterListText->SetText(NSLOCTEXT("DBACharacterSelectWidget", "NoCharacters", "暂无角色，请创建角色。"));
		return;
	}

	TArray<FString> Lines;
	for (const FDBACharacterSummary& Character : CurrentCharacters)
	{
		const FString Prefix = Character.CharacterId == SelectedCharacterId ? TEXT("> ") : TEXT("  ");
		Lines.Add(FString::Printf(TEXT("%s%s  等级%d"), *Prefix, *Character.CharacterName, Character.Level));
	}

	const FDBACharacterSummary* SelectedCharacter = CurrentCharacters.FindByPredicate([this](const FDBACharacterSummary& Character)
	{
		return Character.CharacterId == SelectedCharacterId;
	});
	FText PresentationText;
	if (SelectedCharacter && ZodiacPresentationData && ZodiacPresentationData->GetCharacterSelectionSummaryText(SelectedCharacter->Zodiac, SelectedCharacter->PrimaryElement, PresentationText))
	{
		Lines.Add(PresentationText.ToString());
	}
	CharacterListText->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
}

void UDBACharacterSelectFlowWidgetBase::InitializeZodiacPresentationData()
{
	const UDBAUIDeveloperSettings* UISettings = GetDefault<UDBAUIDeveloperSettings>();
	if (!UISettings || UISettings->ZodiacCharacterSelectionData.IsNull())
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[角色选择界面] 未配置十二生肖角色数据资产。"));
		return;
	}

	TWeakObjectPtr<UDBACharacterSelectFlowWidgetBase> WeakThis(this);
	DBAAsyncAssetLoader::RequestAsyncAsset<UDBAZodiacHeroDataAsset>(this, UISettings->ZodiacCharacterSelectionData, [WeakThis](UDBAZodiacHeroDataAsset* LoadedAsset)
	{
		if (!WeakThis.IsValid() || !LoadedAsset)
		{
			return;
		}

		WeakThis->ZodiacPresentationData = LoadedAsset;
		LoadedAsset->OnDataTableLoaded.AddUObject(WeakThis.Get(), &UDBACharacterSelectFlowWidgetBase::HandleZodiacPresentationDataTableLoaded);
		LoadedAsset->PreloadAllDataTablesAsync();
		WeakThis->RefreshCharacterText();
	});
}

void UDBACharacterSelectFlowWidgetBase::HandleZodiacPresentationDataTableLoaded(UDataTable* LoadedTable, const FSoftObjectPath& AssetPath)
{
	UE_LOG(LogDBAUI, Verbose, TEXT("[角色选择界面] 十二生肖展示数据已加载：%s"), *AssetPath.ToString());
	RefreshCharacterText();
}

void UDBACharacterSelectFlowWidgetBase::SetStatus(const FText& InStatusText)
{
	if (StatusText)
	{
		StatusText->SetText(InStatusText);
		StatusText->SetVisibility(InStatusText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

UDBAFrontendFlowSubsystem* UDBACharacterSelectFlowWidgetBase::GetLoginFlow() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendFlowSubsystem>() : nullptr;
}

UDBAFrontendFlowController* UDBACharacterSelectFlowWidgetBase::GetFrontendFlowController() const
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

void UDBACharacterSelectFlowWidgetBase::InitializePresentationLevel()
{
	if (InjectedBackgroundImage)
	{
		InjectedBackgroundImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
		InjectedBackgroundImage->SetRenderOpacity(0.0f);
		InjectedBackgroundImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	HideSelectWorldStageBackgroundImages(WidgetTree);

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
		// 前端控制器 Possess/Acknowledge 可能抢回 ViewTarget；短时重复拉回舞台相机避免闪一下黑屏。
		DeferredPresentationActivateRetryCount = 0;
		World->GetTimerManager().ClearTimer(DeferredPresentationActivateTimerHandle);
		World->GetTimerManager().SetTimer(
			DeferredPresentationActivateTimerHandle,
			this,
			&UDBACharacterSelectFlowWidgetBase::HandleDeferredPresentationStageBinding,
			0.05f,
			true);
	}
}

void UDBACharacterSelectFlowWidgetBase::BindPlacedPresentationStage()
{
	SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	SetRenderOpacity(1.0f);

	if (!PresentationStage)
	{
		PresentationStage = ADBACharacterPresentationActor::FindPlacedPresentationStage(GetWorld());
	}

	ADBACharacterPreviewStage* PreviewStage = ADBACharacterPreviewStage::FindPlacedPreviewStage(GetWorld());
	if (!PresentationStage && !PreviewStage)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[角色选择界面] 固定关卡中未放置 PreviewStage 或兼容展示舞台，无法显示角色。"));
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC && GetWorld())
	{
		PC = GetWorld()->GetFirstPlayerController();
	}

	if (PresentationStage)
	{
		PresentationStage->SetActorHiddenInGame(false);
	}
	if (UDBACharacterPreviewSubsystem* PreviewSubsystem = GetGameInstance()->GetSubsystem<UDBACharacterPreviewSubsystem>())
	{
		PreviewSubsystem->ActivateCamera(PC, 0.0f);
	}

	if (SelectedCharacterId.IsValid())
	{
		UpdatePresentedCharacterById(SelectedCharacterId);
	}
	else if (CurrentCharacters.Num() > 0)
	{
		UpdatePresentedCharacterById(CurrentCharacters[0].CharacterId);
	}
	else
	{
		UE_LOG(LogDBAUI, Log, TEXT("[角色选择界面] 当前没有角色，展示舞台不加载任何生肖预览资源。"));
	}

	AActor* ViewTarget = PC ? PC->GetViewTarget() : nullptr;
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] 使用世界 3D 角色展示舞台。ViewTarget=%s 舞台=%s"),
		ViewTarget ? *ViewTarget->GetName() : TEXT("无"),
		*GetNameSafe(PresentationStage ? static_cast<AActor*>(PresentationStage) : static_cast<AActor*>(PreviewStage)));
}

void UDBACharacterSelectFlowWidgetBase::HandleDeferredPresentationStageBinding()
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

void UDBACharacterSelectFlowWidgetBase::ReleasePresentationLevel()
{
	// 固定关卡舞台由关卡生命周期管理，界面销毁时仅释放弱关联。
	PresentationStage = nullptr;
}

bool UDBACharacterSelectFlowWidgetBase::IsPointerOverInteractiveControl(const FVector2D& ScreenPosition) const
{
	const TArray<const UWidget*> InteractiveControls = {
		ConfirmButton,
		CreateButton,
		RefreshButton,
		PrevCharacterButton,
		NextCharacterButton
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

void UDBACharacterSelectFlowWidgetBase::BeginPreviewRotationDrag(const FVector2D& ScreenPosition)
{
	if (!PresentationStage)
	{
		return;
	}

	bIsPreviewRotationDragging = true;
	LastPreviewDragScreenPosition = ScreenPosition;
}

void UDBACharacterSelectFlowWidgetBase::UpdatePreviewRotationDrag(const FVector2D& ScreenPosition)
{
	if (!bIsPreviewRotationDragging)
	{
		return;
	}

	const float DeltaX = ScreenPosition.X - LastPreviewDragScreenPosition.X;
	if (FMath::Abs(DeltaX) > KINDA_SMALL_NUMBER)
	{
		if (UDBACharacterPreviewSubsystem* PreviewSubsystem = GetGameInstance()->GetSubsystem<UDBACharacterPreviewSubsystem>())
		{
			PreviewSubsystem->Rotate(DeltaX * PreviewDragRotationDegreesPerPixel);
		}
	}

	LastPreviewDragScreenPosition = ScreenPosition;
}

void UDBACharacterSelectFlowWidgetBase::EndPreviewRotationDrag()
{
	bIsPreviewRotationDragging = false;
}

void UDBACharacterSelectFlowWidgetBase::UpdatePresentedCharacterById(const FDBACharacterId& CharacterId)
{
	const FDBACharacterSummary* SelectedSummary = CurrentCharacters.FindByPredicate(
		[&CharacterId](const FDBACharacterSummary& Character)
		{
			return Character.CharacterId == CharacterId;
		});

	EDBAZodiac PreviewZodiac = EDBAZodiac::None;
	if (SelectedSummary && SelectedSummary->Zodiac != EDBAZodiac::None)
	{
		PreviewZodiac = SelectedSummary->Zodiac;
	}
	else if (CurrentCharacters.Num() > 0 && CurrentCharacters[0].Zodiac != EDBAZodiac::None)
	{
		PreviewZodiac = CurrentCharacters[0].Zodiac;
	}

	if (PreviewZodiac != EDBAZodiac::None)
	{
		if (UDBACharacterPreviewSubsystem* PreviewSubsystem = GetGameInstance()->GetSubsystem<UDBACharacterPreviewSubsystem>())
		{
			const UDBACharacterRosterSubsystem* Roster = GetGameInstance()->GetSubsystem<UDBACharacterRosterSubsystem>();
			const FDBACharacterDetails* Details = Roster ? Roster->FindCachedCharacter(CharacterId) : nullptr;
			PreviewSubsystem->SelectCharacter(PreviewZodiac, Details ? Details->Appearance : FDBACharacterAppearance());
		}
	}
}

void UDBACharacterSelectFlowWidgetBase::HandleBackgroundMusicFinished()
{
	StartBackgroundMusic();
}

void UDBACharacterSelectFlowWidgetBase::InitializeAudioAssets()
{
	const UDBAUIDeveloperSettings* UISettings = GetDefault<UDBAUIDeveloperSettings>();
	if (!UISettings)
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterSelectWidget] 未找到 UI 音频配置。"));
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
		DBAAsyncAssetLoader::RequestAsyncAsset<USoundBase>(this, UISettings->CharacterSelectBGM,
			[this](USoundBase* LoadedSound)
			{
				BackgroundMusicSound = LoadedSound;
			});
	}
}

void UDBACharacterSelectFlowWidgetBase::StartBackgroundMusic()
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
		BackgroundMusicComponent->OnAudioFinished.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleBackgroundMusicFinished);
		BackgroundMusicComponent->OnAudioFinished.AddDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleBackgroundMusicFinished);
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterSelectWidget] 创建背景音乐组件失败。"));
	}
}

void UDBACharacterSelectFlowWidgetBase::StopBackgroundMusic()
{
	if (!BackgroundMusicComponent)
	{
		return;
	}

	BackgroundMusicComponent->OnAudioFinished.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleBackgroundMusicFinished);
	BackgroundMusicComponent->Stop();
	BackgroundMusicComponent = nullptr;
}

void UDBACharacterSelectFlowWidgetBase::PlayButtonClickSfx() const
{
	if (ButtonClickSound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ButtonClickSound, 0.85f);
	}
}
