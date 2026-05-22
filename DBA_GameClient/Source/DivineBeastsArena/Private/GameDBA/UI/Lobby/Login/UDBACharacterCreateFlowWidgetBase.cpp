// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/AudioComponent.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ContentWidget.h"
#include "Components/EditableTextBox.h"
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

	UWidget* ResolveCreatePreviewHost(UWidgetTree* WidgetTree, UWidget* CurrentHost)
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

	bool IsDedicatedCreatePreviewHost(const UWidget* HostWidget)
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

	bool IsBlockedCharacterCreateNavigationKey(const FKey& Key)
	{
		return Key == EKeys::Left || Key == EKeys::Right;
	}

	void ConfigureCreateOverlaySlot(UOverlaySlot* Slot)
	{
		if (!Slot)
		{
			return;
		}

		Slot->SetHorizontalAlignment(HAlign_Fill);
		Slot->SetVerticalAlignment(VAlign_Fill);
	}

	void AddCreatePreviewStageImage(UWidgetTree* WidgetTree, UOverlay* Overlay, const FName WidgetName, const TCHAR* TexturePath, float Opacity)
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
		ConfigureCreateOverlaySlot(Overlay->AddChildToOverlay(StageImage));
	}

	bool TryAttachCreatePreviewViewport(UWidgetTree* WidgetTree, UWidget* HostWidget, UViewport* Viewport)
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
			if (IsDedicatedCreatePreviewHost(HostWidget))
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

		AddCreatePreviewStageImage(
			WidgetTree,
			PreviewStageOverlay,
			TEXT("CharacterPreviewBackdrop_Auto"),
			TEXT("/Game/DBA/UI/Lobby/Character/Textures/T_DBA_PreviewStage_Backdrop.T_DBA_PreviewStage_Backdrop"),
			1.0f);

		Viewport->SetRenderOpacity(0.88f);
		Viewport->SetVisibility(ESlateVisibility::HitTestInvisible);
		ConfigureCreateOverlaySlot(PreviewStageOverlay->AddChildToOverlay(Viewport));

		AddCreatePreviewStageImage(
			WidgetTree,
			PreviewStageOverlay,
			TEXT("CharacterPreviewForeground_Auto"),
			TEXT("/Game/DBA/UI/Lobby/Character/Textures/T_DBA_PreviewStage_Foreground.T_DBA_PreviewStage_Foreground"),
			1.0f);

		return true;
	}

	void ApplyCreateHostTransparency(UWidget* HostWidget)
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

	bool IsCreateBackgroundImageName(const FString& Name)
	{
		const FString LowerName = Name.ToLower();
		return LowerName.Contains(TEXT("previewstage"))
			|| LowerName.Contains(TEXT("characterpreviewbg"))
			|| LowerName.Contains(TEXT("characterpreviewbackdrop"))
			|| LowerName.Contains(TEXT("characterpreviewforeground"));
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

		WidgetTree->ForEachWidgetAndDescendants(
			[](UWidget* Widget)
			{
				UImage* Image = Cast<UImage>(Widget);
				if (Image && IsCreateBackgroundImageName(Image->GetName()))
				{
					Image->SetRenderOpacity(0.0f);
					Image->SetVisibility(ESlateVisibility::HitTestInvisible);
				}
			});
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

UDBACharacterCreateFlowWidgetBase::UDBACharacterCreateFlowWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
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
	InitializePreviewViewport();
	RefreshChoiceText();
	Validate();

	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleFlowError);
		LoginFlow->OnFlowError.AddDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleFlowError);
	}
}

void UDBACharacterCreateFlowWidgetBase::NativeDestruct()
{
	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBACharacterCreateFlowWidgetBase::HandleFlowError);
	}

	UnbindControls();
	DestroyPreviewViewport();
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

FReply UDBACharacterCreateFlowWidgetBase::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (IsPointerOverPreviewHost(InGestureEvent.GetScreenSpacePosition()))
	{
		BeginPreviewRotationDrag(InGestureEvent.GetScreenSpacePosition());
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
	Validate();
}

void UDBACharacterCreateFlowWidgetBase::SetZodiac(EDBAZodiac Zodiac)
{
	SelectedZodiac = Zodiac == EDBAZodiac::None ? EDBAZodiac::Rat : Zodiac;
	RefreshChoiceText();
	RefreshPreviewCharacter();
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
	if (CharacterNameInput)
	{
		CharacterName = CharacterNameInput->GetText().ToString().TrimStartAndEnd();
	}

	if (!Validate())
	{
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

	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		ShowValidationMessage(true, NSLOCTEXT("DBACharacterCreateWidget", "Creating", "正在创建角色..."));
		if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
			{
				UIManager->ShowLobbyLoadingScreen();
			}
		}
		LoginFlow->SubmitCharacterCreation(Request);
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] Submitted character creation: %s"), *CharacterName);
	}
	else
	{
		ShowValidationMessage(false, NSLOCTEXT("DBACharacterCreateWidget", "FlowUnavailable", "登录流程不可用。"));
	}
}

void UDBACharacterCreateFlowWidgetBase::BackToCharacterSelect()
{
	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->BackToCharacterSelect();
	}
}

void UDBACharacterCreateFlowWidgetBase::HandleCreateClicked()
{
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

void UDBACharacterCreateFlowWidgetBase::HandleFlowError(const FString& ErrorMessage)
{
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

	CharacterPreviewViewport = WidgetTree->ConstructWidget<UViewport>(UViewport::StaticClass(), TEXT("CharacterPreviewViewport"));
	RootBox->AddChildToVerticalBox(CharacterPreviewViewport);

	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] Native fallback layout created"));
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

	if (!CharacterNameInput)
	{
		CharacterNameInput = Cast<UEditableTextBox>(FindNamedWidget({ TEXT("CharacterNameInput"), TEXT("NameInput"), TEXT("RoleNameInput") }));
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
	if (!CharacterPreviewHost)
	{
		CharacterPreviewHost = FindNamedWidget({ TEXT("CharacterPreviewHost"), TEXT("PreviewHost"), TEXT("PreviewContainer"), TEXT("CharacterPreviewContainer") });
	}
	if (!CharacterPreviewViewport)
	{
		CharacterPreviewViewport = Cast<UViewport>(FindNamedWidget({ TEXT("CharacterPreviewViewport"), TEXT("PreviewViewport") }));
	}
}

void UDBACharacterCreateFlowWidgetBase::ApplyBlueprintLayoutOverrides()
{
	ApplyWidgetSlot(
		CharacterPreviewHost,
		FAnchors(0.24f, 0.05f, 0.76f, 0.95f),
		FMargin(0.0f, 0.0f, 0.0f, 0.0f),
		FVector2D(0.0f, 0.0f));

	ApplyWidgetSlot(
		CharacterNameInput,
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

	SetCreateTextBlockByNames(WidgetTree, { TEXT("NativeCreateTitle"), TEXT("TitleText"), TEXT("CharacterCreateTitle"), TEXT("CreateTitle") }, TitleText);
	SetCreateTextBlockByNames(WidgetTree, { TEXT("CreateButtonText"), TEXT("ConfirmCreateButtonText") }, CreateText);
	SetCreateTextBlockByNames(WidgetTree, { TEXT("BackButtonText"), TEXT("BackToSelectText"), TEXT("BackToSelectButtonText") }, BackText);

	SetCreateButtonLabel(CreateButton, CreateText);
	SetCreateButtonLabel(BackButton, BackText);

	ReplaceCreateTextBlockValue(WidgetTree, TEXT("Create Character"), TitleText);
	ReplaceCreateTextBlockValue(WidgetTree, TEXT("Create and Enter"), CreateText);
	ReplaceCreateTextBlockValue(WidgetTree, TEXT("Back to Character Select"), BackText);
}

void UDBACharacterCreateFlowWidgetBase::BindControls()
{
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
	if (CharacterNameInput)
	{
		CharacterName = CharacterNameInput->GetText().ToString().TrimStartAndEnd();
	}

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
	if (NameLen < 2 || NameLen > 14)
	{
		OutMessage = NSLOCTEXT("DBACharacterCreateWidget", "NameLen", "名称长度需为 2-14 个字符。");
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
		ZodiacText->SetText(FText::Format(NSLOCTEXT("DBACharacterCreateWidget", "ZodiacFormat", "生肖：{0}"), EnumValueText(SelectedZodiac)));
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

void UDBACharacterCreateFlowWidgetBase::ShowValidationMessage(bool bValid, const FText& Message)
{
	if (ValidationText)
	{
		ValidationText->SetText(Message);
		ValidationText->SetVisibility(Message.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	BP_OnValidationChanged(bValid, Message);
}

UDBALoginFlowSubsystem* UDBACharacterCreateFlowWidgetBase::GetLoginFlow() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>() : nullptr;
}

void UDBACharacterCreateFlowWidgetBase::InitializePreviewViewport()
{
	if (!CharacterPreviewHost)
	{
		CharacterPreviewHost = ResolveCreatePreviewHost(WidgetTree, CharacterPreviewHost);
		ApplyCreateHostTransparency(CharacterPreviewHost);
	}

	HideCreateWorldStageBackgroundImages(WidgetTree);

	if (CharacterPreviewHost)
	{
		CharacterPreviewHost->SetRenderOpacity(0.0f);
		CharacterPreviewHost->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (PreviewActor)
	{
		PreviewActor->ActivatePresentationCamera(GetOwningPlayer());
		PreviewActor->SetPreviewZodiac(SelectedZodiac);
		return;
	}

	PreviewActor = ADBACharacterPresentationActor::ResolveSharedPresentationStage(GetWorld());
	if (PreviewActor)
	{
		PreviewActor->ActivatePresentationCamera(GetOwningPlayer());
		PreviewActor->SetPreviewZodiac(SelectedZodiac);
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] Using world 3D character presentation stage for zodiac %d."), static_cast<int32>(SelectedZodiac));
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterCreateWidget] Failed to spawn world 3D character presentation stage."));
	}
}

void UDBACharacterCreateFlowWidgetBase::DestroyPreviewViewport()
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

bool UDBACharacterCreateFlowWidgetBase::IsPointerOverPreviewHost(const FVector2D& ScreenPosition) const
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

void UDBACharacterCreateFlowWidgetBase::BeginPreviewRotationDrag(const FVector2D& ScreenPosition)
{
	if (!PreviewActor)
	{
		return;
	}

	bIsPreviewRotationDragging = true;
	LastPreviewDragScreenPosition = ScreenPosition;
}

void UDBACharacterCreateFlowWidgetBase::UpdatePreviewRotationDrag(const FVector2D& ScreenPosition)
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

void UDBACharacterCreateFlowWidgetBase::EndPreviewRotationDrag()
{
	bIsPreviewRotationDragging = false;
}

void UDBACharacterCreateFlowWidgetBase::RefreshPreviewCharacter()
{
	if (PreviewActor)
	{
		PreviewActor->SetPreviewZodiac(SelectedZodiac);
	}
}

void UDBACharacterCreateFlowWidgetBase::HandleBackgroundMusicFinished()
{
	StartBackgroundMusic();
}

void UDBACharacterCreateFlowWidgetBase::InitializeAudioAssets()
{
	if (!ButtonClickSound)
	{
		ButtonClickSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick.SFX_UI_ButtonClick"));
		if (!ButtonClickSound)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[CharacterCreateWidget] Button click sound not found."));
		}
	}

	if (!BackgroundMusicSound)
	{
		BackgroundMusicSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/DBA/Audio/UI/BGM/BGM_CharacterCreate_Loop.BGM_CharacterCreate_Loop"));
		if (!BackgroundMusicSound)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[CharacterCreateWidget] BGM asset not found."));
		}
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
		UE_LOG(LogDBAUI, Warning, TEXT("[CharacterCreateWidget] Failed to spawn BGM component."));
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
