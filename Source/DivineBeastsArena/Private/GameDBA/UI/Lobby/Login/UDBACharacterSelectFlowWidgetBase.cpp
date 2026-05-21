// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.h"

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
#include "Components/Viewport.h"
#include "Components/VerticalBox.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/Texture2D.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameDBA/UI/DBAGameUIManager.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/DBAUIFontUtils.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPreviewActor.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

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

	UWidget* ResolveSelectPreviewHost(UWidgetTree* WidgetTree, UWidget* CurrentHost)
	{
		if (!WidgetTree)
		{
			return nullptr;
		}

		if (CurrentHost)
		{
			return CurrentHost;
		}

		const TArray<FName> CandidateNames = {
			TEXT("CharacterPreviewHost"),
			TEXT("PreviewHost"),
			TEXT("PreviewContainer"),
			TEXT("CharacterPreviewContainer")
		};

		for (const FName Candidate : CandidateNames)
		{
			if (UWidget* FoundHost = WidgetTree->FindWidget(Candidate))
			{
				return FoundHost;
			}
		}

		return nullptr;
	}

	bool IsDedicatedSelectPreviewHost(const UWidget* HostWidget)
	{
		if (!HostWidget)
		{
			return false;
		}

		const FName HostName = HostWidget->GetFName();
		return HostName == TEXT("CharacterPreviewHost")
			|| HostName == TEXT("PreviewHost")
			|| HostName == TEXT("PreviewContainer")
			|| HostName == TEXT("CharacterPreviewContainer");
	}

	bool IsBlockedCharacterSelectNavigationKey(const FKey& Key)
	{
		return Key == EKeys::Left || Key == EKeys::Right;
	}

	void ConfigureSelectOverlaySlot(UOverlaySlot* Slot)
	{
		if (!Slot)
		{
			return;
		}

		Slot->SetHorizontalAlignment(HAlign_Fill);
		Slot->SetVerticalAlignment(VAlign_Fill);
	}

	void AddSelectPreviewStageImage(UWidgetTree* WidgetTree, UOverlay* Overlay, const FName WidgetName, const TCHAR* TexturePath, float Opacity)
	{
		if (!WidgetTree || !Overlay)
		{
			return;
		}

		UImage* StageImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), WidgetName);
		if (!StageImage)
		{
			return;
		}

		if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, TexturePath))
		{
			StageImage->SetBrushFromTexture(Texture, true);
		}

		StageImage->SetRenderOpacity(Opacity);
		StageImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		ConfigureSelectOverlaySlot(Overlay->AddChildToOverlay(StageImage));
	}

	bool TryAttachSelectPreviewViewport(UWidgetTree* WidgetTree, UWidget* HostWidget, UViewport* Viewport)
	{
		if (!WidgetTree || !HostWidget || !Viewport)
		{
			return false;
		}

		if (Viewport->GetParent())
		{
			Viewport->RemoveFromParent();
		}

		UOverlay* PreviewStageOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("CharacterPreviewStageOverlay_Auto"));
		if (!PreviewStageOverlay)
		{
			return false;
		}
		PreviewStageOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		if (UPanelWidget* PanelHost = Cast<UPanelWidget>(HostWidget))
		{
			if (IsDedicatedSelectPreviewHost(HostWidget))
			{
				PanelHost->ClearChildren();
			}
			PanelHost->AddChild(PreviewStageOverlay);
		}
		else if (UContentWidget* ContentHost = Cast<UContentWidget>(HostWidget))
		{
			ContentHost->SetContent(PreviewStageOverlay);
		}
		else
		{
			return false;
		}

		AddSelectPreviewStageImage(
			WidgetTree,
			PreviewStageOverlay,
			TEXT("CharacterPreviewBackdrop_Auto"),
			TEXT("/Game/DBA/UI/Lobby/Character/Textures/T_DBA_PreviewStage_Backdrop.T_DBA_PreviewStage_Backdrop"),
			1.0f);

		Viewport->SetRenderOpacity(0.88f);
		Viewport->SetVisibility(ESlateVisibility::HitTestInvisible);
		ConfigureSelectOverlaySlot(PreviewStageOverlay->AddChildToOverlay(Viewport));

		AddSelectPreviewStageImage(
			WidgetTree,
			PreviewStageOverlay,
			TEXT("CharacterPreviewForeground_Auto"),
			TEXT("/Game/DBA/UI/Lobby/Character/Textures/T_DBA_PreviewStage_Foreground.T_DBA_PreviewStage_Foreground"),
			1.0f);

		return true;
	}

	void ApplySelectHostTransparency(UWidget* HostWidget)
	{
		if (UBorder* Border = Cast<UBorder>(HostWidget))
		{
			FLinearColor Color = Border->GetBrushColor();
			Color.A = 0.0f;
			Border->SetBrushColor(Color);
		}

		if (UImage* Image = Cast<UImage>(HostWidget))
		{
			Image->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
		}

		if (UButton* Button = Cast<UButton>(HostWidget))
		{
			Button->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
			Button->SetBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
		}
	}

	bool IsSelectBackgroundImageName(const FString& Name)
	{
		const FString LowerName = Name.ToLower();
		return LowerName.Contains(TEXT("previewstage"))
			|| LowerName.Contains(TEXT("characterpreviewbg"))
			|| LowerName.Contains(TEXT("characterpreviewbackdrop"))
			|| LowerName.Contains(TEXT("characterpreviewforeground"));
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

		WidgetTree->ForEachWidgetAndDescendants(
			[](UWidget* Widget)
			{
				UImage* Image = Cast<UImage>(Widget);
				if (Image && IsSelectBackgroundImageName(Image->GetName()))
				{
					Image->SetRenderOpacity(0.0f);
					Image->SetVisibility(ESlateVisibility::HitTestInvisible);
				}
			});
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
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
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
	BindControls();
	InitializeAudioAssets();
	DBAUIFonts::ApplyGameFontToWidgetTree(WidgetTree);
	ApplySelectMenuInputMode(this);
	InitializePreviewViewport();

	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
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
	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->OnCharactersLoaded.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleCharactersLoaded);
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBACharacterSelectFlowWidgetBase::HandleFlowError);
	}

	UnbindControls();
	DestroyPreviewViewport();
	Super::NativeDestruct();
}

FReply UDBACharacterSelectFlowWidgetBase::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (IsBlockedCharacterSelectNavigationKey(InKeyEvent.GetKey()))
	{
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UDBACharacterSelectFlowWidgetBase::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (IsBlockedCharacterSelectNavigationKey(InKeyEvent.GetKey()))
	{
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UDBACharacterSelectFlowWidgetBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && IsPointerOverPreviewHost(InMouseEvent.GetScreenSpacePosition()))
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
		if (PreviewActor && FMath::Abs(MouseDelta.X) > KINDA_SMALL_NUMBER)
		{
			PreviewActor->AddPreviewYaw(MouseDelta.X * PreviewDragRotationDegreesPerPixel);
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
	if (IsPointerOverPreviewHost(InGestureEvent.GetScreenSpacePosition()))
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
	if (!SelectedCharacterId.IsValid() && CurrentCharacters.Num() > 0)
	{
		SelectedCharacterId = CurrentCharacters[0].CharacterId;
	}

	RefreshCharacterText();
	UpdateCharacterPreviewById(SelectedCharacterId);
	BP_OnCharactersUpdated(Characters);
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] Characters updated: %d"), Characters.Num());
}

void UDBACharacterSelectFlowWidgetBase::SelectCharacter(const FDBACharacterId& CharacterId)
{
	SelectedCharacterId = CharacterId;
	BP_OnCharacterSelected(CharacterId);
	RefreshCharacterText();
	UpdateCharacterPreviewById(CharacterId);
	UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] Selected character: %s"), *CharacterId.ToString());
}

void UDBACharacterSelectFlowWidgetBase::ConfirmSelectedCharacter()
{
	if (!SelectedCharacterId.IsValid())
	{
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "NoSelection", "尚未选择角色。"));
		return;
	}

	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "EnteringLobby", "正在进入大厅..."));
		if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
			{
				UIManager->ShowLobbyLoadingScreen();
			}
		}
		LoginFlow->SubmitCharacterSelection(SelectedCharacterId);
	}
	else
	{
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "FlowUnavailable", "登录流程不可用。"));
	}
}

void UDBACharacterSelectFlowWidgetBase::EnterCharacterCreate()
{
	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->EnterCharacterCreate();
	}
}

void UDBACharacterSelectFlowWidgetBase::RefreshCharacterList()
{
	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "Refreshing", "正在刷新角色列表..."));
		LoginFlow->RefreshCharacterList();
	}
}

void UDBACharacterSelectFlowWidgetBase::HandleConfirmClicked()
{
	PlayButtonClickSfx();
	ConfirmSelectedCharacter();
}

void UDBACharacterSelectFlowWidgetBase::HandleCreateClicked()
{
	PlayButtonClickSfx();
	EnterCharacterCreate();
}

void UDBACharacterSelectFlowWidgetBase::HandleRefreshClicked()
{
	PlayButtonClickSfx();
	RefreshCharacterList();
}

void UDBACharacterSelectFlowWidgetBase::HandleCharactersLoaded(const TArray<FDBACharacterSummary>& Characters)
{
	UpdateCharacters(Characters);
}

void UDBACharacterSelectFlowWidgetBase::HandleFlowError(const FString& ErrorMessage)
{
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

	CharacterPreviewViewport = WidgetTree->ConstructWidget<UViewport>(UViewport::StaticClass(), TEXT("CharacterPreviewViewport"));
	RootBox->AddChildToVerticalBox(CharacterPreviewViewport);

	UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] Native fallback layout created"));
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
	if (!CharacterPreviewHost)
	{
		CharacterPreviewHost = FindNamedWidget({ TEXT("CharacterPreviewHost"), TEXT("PreviewHost"), TEXT("PreviewContainer"), TEXT("CharacterPreviewContainer") });
	}
	if (!CharacterPreviewViewport)
	{
		CharacterPreviewViewport = Cast<UViewport>(FindNamedWidget({ TEXT("CharacterPreviewViewport"), TEXT("PreviewViewport") }));
	}
}

void UDBACharacterSelectFlowWidgetBase::ApplyBlueprintLayoutOverrides()
{
	ApplySelectWidgetSlot(
		CharacterPreviewHost,
		FAnchors(0.24f, 0.05f, 0.76f, 0.95f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

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

	SetSelectTextBlockByNames(WidgetTree, { TEXT("NativeSelectTitle"), TEXT("TitleText"), TEXT("CharacterSelectTitle"), TEXT("SelectTitle") }, TitleText);
	SetSelectTextBlockByNames(WidgetTree, { TEXT("ConfirmButtonText"), TEXT("EnterLobbyText"), TEXT("EnterText") }, ConfirmText);
	SetSelectTextBlockByNames(WidgetTree, { TEXT("CreateButtonText"), TEXT("CreateCharacterText") }, CreateText);
	SetSelectTextBlockByNames(WidgetTree, { TEXT("RefreshButtonText"), TEXT("RefreshListText"), TEXT("ReloadButtonText") }, RefreshText);

	SetSelectButtonLabel(ConfirmButton, ConfirmText);
	SetSelectButtonLabel(CreateButton, CreateText);
	SetSelectButtonLabel(RefreshButton, RefreshText);

	ReplaceSelectTextBlockValue(WidgetTree, TEXT("Select Character"), TitleText);
	ReplaceSelectTextBlockValue(WidgetTree, TEXT("Enter Lobby"), ConfirmText);
	ReplaceSelectTextBlockValue(WidgetTree, TEXT("Create Character"), CreateText);
	ReplaceSelectTextBlockValue(WidgetTree, TEXT("Refresh"), RefreshText);
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
	CharacterListText->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
}

void UDBACharacterSelectFlowWidgetBase::SetStatus(const FText& InStatusText)
{
	if (StatusText)
	{
		StatusText->SetText(InStatusText);
		StatusText->SetVisibility(InStatusText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

UDBALoginFlowSubsystem* UDBACharacterSelectFlowWidgetBase::GetLoginFlow() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>() : nullptr;
}

void UDBACharacterSelectFlowWidgetBase::InitializePreviewViewport()
{
	if (!CharacterPreviewHost)
	{
		CharacterPreviewHost = ResolveSelectPreviewHost(WidgetTree, CharacterPreviewHost);
		ApplySelectHostTransparency(CharacterPreviewHost);
	}

	HideSelectWorldStageBackgroundImages(WidgetTree);

	if (CharacterPreviewHost)
	{
		CharacterPreviewHost->SetRenderOpacity(0.0f);
		CharacterPreviewHost->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (PreviewActor)
	{
		PreviewActor->ActivatePresentationCamera(GetOwningPlayer());
		return;
	}

	PreviewActor = ADBACharacterPresentationActor::ResolveSharedPresentationStage(GetWorld());
	if (PreviewActor)
	{
		PreviewActor->ActivatePresentationCamera(GetOwningPlayer());
		UpdateCharacterPreviewById(SelectedCharacterId);
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] Using world 3D character presentation stage."));
	}
}

void UDBACharacterSelectFlowWidgetBase::DestroyPreviewViewport()
{
	if (PreviewActor)
	{
		ADBACharacterPresentationActor* SharedStage = PreviewActor.Get();
		ADBACharacterPresentationActor::ReleaseSharedPresentationStage(SharedStage);
		PreviewActor = SharedStage;
	}
	if (PreviewDirectionalLight)
	{
		PreviewDirectionalLight->Destroy();
		PreviewDirectionalLight = nullptr;
	}
	if (PreviewFillLight)
	{
		PreviewFillLight->Destroy();
		PreviewFillLight = nullptr;
	}
	if (PreviewSkyLight)
	{
		PreviewSkyLight->Destroy();
		PreviewSkyLight = nullptr;
	}
}

bool UDBACharacterSelectFlowWidgetBase::IsPointerOverPreviewHost(const FVector2D& ScreenPosition) const
{
	if (CharacterPreviewHost && CharacterPreviewHost->GetCachedGeometry().IsUnderLocation(ScreenPosition))
	{
		return true;
	}

	const FGeometry& RootGeometry = GetCachedGeometry();
	const FVector2D LocalPosition = RootGeometry.AbsoluteToLocal(ScreenPosition);
	const FVector2D LocalSize = RootGeometry.GetLocalSize();
	return LocalSize.X > 1.0f
		&& LocalSize.Y > 1.0f
		&& LocalPosition.X >= LocalSize.X * 0.24f
		&& LocalPosition.X <= LocalSize.X * 0.76f
		&& LocalPosition.Y >= LocalSize.Y * 0.05f
		&& LocalPosition.Y <= LocalSize.Y * 0.95f;
}

void UDBACharacterSelectFlowWidgetBase::BeginPreviewRotationDrag(const FVector2D& ScreenPosition)
{
	if (!PreviewActor)
	{
		return;
	}

	bIsPreviewRotationDragging = true;
	LastPreviewDragScreenPosition = ScreenPosition;
}

void UDBACharacterSelectFlowWidgetBase::UpdatePreviewRotationDrag(const FVector2D& ScreenPosition)
{
	if (!bIsPreviewRotationDragging || !PreviewActor)
	{
		return;
	}

	const float DeltaX = ScreenPosition.X - LastPreviewDragScreenPosition.X;
	if (FMath::Abs(DeltaX) > KINDA_SMALL_NUMBER)
	{
		PreviewActor->AddPreviewYaw(DeltaX * PreviewDragRotationDegreesPerPixel);
	}

	LastPreviewDragScreenPosition = ScreenPosition;
}

void UDBACharacterSelectFlowWidgetBase::EndPreviewRotationDrag()
{
	bIsPreviewRotationDragging = false;
}

void UDBACharacterSelectFlowWidgetBase::UpdateCharacterPreviewById(const FDBACharacterId& CharacterId)
{
	if (!PreviewActor)
	{
		return;
	}

	const FDBACharacterSummary* SelectedSummary = CurrentCharacters.FindByPredicate(
		[&CharacterId](const FDBACharacterSummary& Character)
		{
			return Character.CharacterId == CharacterId;
		});

	if (SelectedSummary)
	{
		PreviewActor->SetPreviewZodiac(SelectedSummary->Zodiac);
	}
}

void UDBACharacterSelectFlowWidgetBase::HandleBackgroundMusicFinished()
{
	StartBackgroundMusic();
}

void UDBACharacterSelectFlowWidgetBase::InitializeAudioAssets()
{
	if (!ButtonClickSound)
	{
		ButtonClickSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick.SFX_UI_ButtonClick"));
		if (!ButtonClickSound)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[CharacterSelectWidget] Button click sound not found."));
		}
	}

	if (!BackgroundMusicSound)
	{
		BackgroundMusicSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/DBA/Audio/UI/BGM/BGM_CharacterSelect_Loop.BGM_CharacterSelect_Loop"));
		if (!BackgroundMusicSound)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[CharacterSelectWidget] BGM asset not found."));
		}
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
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterSelectWidget] Failed to spawn BGM component."));
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
