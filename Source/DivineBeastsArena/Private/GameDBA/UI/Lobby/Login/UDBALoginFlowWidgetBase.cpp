// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h"

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
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/DBAUIFontUtils.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace
{
	const FLinearColor GoldText(0.95f, 0.77f, 0.36f, 1.0f);
	const FLinearColor DeepJade(0.015f, 0.11f, 0.085f, 0.92f);
	const FLinearColor SoftJade(0.05f, 0.28f, 0.20f, 0.78f);

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

	void ApplyEditableBoxStyle(UEditableTextBox* TextBox)
	{
		if (!TextBox)
		{
			return;
		}

		FEditableTextBoxStyle Style = TextBox->GetWidgetStyle();

		FSlateBrush NormalBrush;
		NormalBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
		NormalBrush.TintColor = FSlateColor(FLinearColor(0.90f, 0.96f, 0.92f, 0.96f));
		Style.SetBackgroundImageNormal(NormalBrush);

		FSlateBrush HoveredBrush = NormalBrush;
		HoveredBrush.TintColor = FSlateColor(FLinearColor(0.95f, 0.99f, 0.96f, 1.0f));
		Style.SetBackgroundImageHovered(HoveredBrush);

		FSlateBrush FocusedBrush = NormalBrush;
		FocusedBrush.TintColor = FSlateColor(FLinearColor(1.0f, 1.0f, 0.92f, 1.0f));
		Style.SetBackgroundImageFocused(FocusedBrush);

		FSlateBrush ReadOnlyBrush = NormalBrush;
		ReadOnlyBrush.TintColor = FSlateColor(FLinearColor(0.66f, 0.70f, 0.68f, 0.92f));
		Style.SetBackgroundImageReadOnly(ReadOnlyBrush);

		Style.ForegroundColor = FSlateColor(FLinearColor(0.09f, 0.12f, 0.10f, 1.0f));
		TextBox->SetWidgetStyle(Style);
		TextBox->SetMinDesiredWidth(500.0f);
		TextBox->SetVisibility(ESlateVisibility::Visible);
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

	FText GetLoginFlowStatusText(EDBALoginFlowState State)
	{
		switch (State)
		{
		case EDBALoginFlowState::TryAutoLogin:
			return NSLOCTEXT("DBALoginFlowWidget", "SigningIn", "Signing in...");
		case EDBALoginFlowState::LoadCharacterList:
			return NSLOCTEXT("DBALoginFlowWidget", "LoadingCharacters", "Loading characters...");
		case EDBALoginFlowState::CharacterSelect:
			return NSLOCTEXT("DBALoginFlowWidget", "ChooseCharacter", "Choose a character.");
		case EDBALoginFlowState::CharacterCreate:
			return NSLOCTEXT("DBALoginFlowWidget", "CreateCharacter", "Create your first character.");
		case EDBALoginFlowState::MainLobby:
			return NSLOCTEXT("DBALoginFlowWidget", "EnteringLobby", "Entering lobby...");
		default:
			return FText::GetEmpty();
		}
	}
}

UDBALoginFlowWidgetBase::UDBALoginFlowWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FDBALoginVisualLayoutSpec UDBALoginFlowWidgetBase::GetReferenceVisualLayoutSpec()
{
	FDBALoginVisualLayoutSpec Spec;
	Spec.PanelAnchorX = 0.66f;
	Spec.TitleText = NSLOCTEXT("DBALoginFlowWidget", "ReferenceTitle", "神兽竞技场");
	Spec.PrimaryButtonText = NSLOCTEXT("DBALoginFlowWidget", "ReferencePrimaryButton", "进入游戏");
	Spec.LeftToolLabels = {
		NSLOCTEXT("DBALoginFlowWidget", "ReferenceAnnouncements", "公告"),
		NSLOCTEXT("DBALoginFlowWidget", "ReferenceRepair", "修复"),
		NSLOCTEXT("DBALoginFlowWidget", "ReferenceSupport", "客服")
	};
	return Spec;
}

void UDBALoginFlowWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBALoginFlowWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeVisualAssets();
	const bool bHasBlueprintLayout = WidgetTree && WidgetTree->RootWidget != nullptr;
	if (bHasBlueprintLayout)
	{
		LoginPanel = FindLoginPanelWidget(WidgetTree);
	}
	else if (bUseReferenceNativeLayout)
	{
		BuildReferenceNativeLayout();
	}
	else
	{
		EnsureNativeFallbackLayout();
	}
	BindControls();
	InitializeAudioAssets();
	ApplyVisualStyle();
	DBAUIFonts::ApplyGameFontToWidgetTree(WidgetTree);
	ClearError();

	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		if (LoginFlow->GetFlowState() == EDBALoginFlowState::Startup)
		{
			LoginFlow->StartLoginFlow();
		}

		LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowStateChanged);
		LoginFlow->OnFlowStateChanged.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowStateChanged);
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowError);
		LoginFlow->OnFlowError.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowError);
		HandleFlowStateChanged(LoginFlow->GetFlowState());
	}
}

void UDBALoginFlowWidgetBase::NativeDestruct()
{
	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		LoginFlow->OnFlowStateChanged.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowStateChanged);
		LoginFlow->OnFlowError.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleFlowError);
	}

	UnbindControls();
	Super::NativeDestruct();
}

void UDBALoginFlowWidgetBase::SubmitLogin()
{
	const FString Email = EmailInput ? EmailInput->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Password = PasswordInput ? PasswordInput->GetText().ToString() : FString();

	if (Email.IsEmpty() || Password.IsEmpty())
	{
		ShowError(TEXT("Please enter email and password."));
		return;
	}

	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		ClearError();
		SetStatus(NSLOCTEXT("DBALoginFlowWidget", "SigningIn", "Signing in..."));
		LoginFlow->SubmitLogin(Email, Password);
	}
	else
	{
		ShowError(TEXT("Login flow unavailable."));
	}
}

void UDBALoginFlowWidgetBase::SubmitGuestLogin()
{
	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		ClearError();
		SetStatus(NSLOCTEXT("DBALoginFlowWidget", "SigningInGuest", "Signing in as guest..."));
		LoginFlow->SubmitGuestLogin();
	}
	else
	{
		ShowError(TEXT("Login flow unavailable."));
	}
}

void UDBALoginFlowWidgetBase::ShowError(const FString& ErrorMessage)
{
	LastErrorMessage = ErrorMessage;
	if (ErrorText)
	{
		ErrorText->SetText(FText::FromString(ErrorMessage));
		ErrorText->SetVisibility(ErrorMessage.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	BP_OnShowError(ErrorMessage);
	UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] Error: %s"), *ErrorMessage);
}

void UDBALoginFlowWidgetBase::ClearError()
{
	LastErrorMessage.Empty();
	if (ErrorText)
	{
		ErrorText->SetText(FText::GetEmpty());
		ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	}
	BP_OnClearError();
}

void UDBALoginFlowWidgetBase::HandleLoginClicked()
{
	PlayButtonClickSfx();
	SubmitLogin();
}

void UDBALoginFlowWidgetBase::HandleGuestLoginClicked()
{
	PlayButtonClickSfx();
	SubmitGuestLogin();
}

void UDBALoginFlowWidgetBase::HandleFlowStateChanged(EDBALoginFlowState NewState)
{
	SetStatus(GetLoginFlowStatusText(NewState));
	if (NewState != EDBALoginFlowState::LoginScreen && NewState != EDBALoginFlowState::Error)
	{
		ClearError();
	}
	BP_OnFlowStateChanged(NewState);
}

void UDBALoginFlowWidgetBase::HandleFlowError(const FString& ErrorMessage)
{
	ShowError(ErrorMessage);
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
	EmailInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "EmailHint", "Email"));
	RootBox->AddChildToVerticalBox(EmailInput);

	PasswordInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("PasswordInput"));
	PasswordInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "PasswordHint", "Password"));
	PasswordInput->SetIsPassword(true);
	RootBox->AddChildToVerticalBox(PasswordInput);

	LoginButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LoginButton"));
	LoginButton->AddChild(MakeLoginButtonLabel(WidgetTree, NSLOCTEXT("DBALoginFlowWidget", "LoginButton", "Login")));
	RootBox->AddChildToVerticalBox(LoginButton);

	GuestLoginButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("GuestLoginButton"));
	GuestLoginButton->AddChild(MakeLoginButtonLabel(WidgetTree, NSLOCTEXT("DBALoginFlowWidget", "GuestButton", "Guest Login")));
	RootBox->AddChildToVerticalBox(GuestLoginButton);

	UE_LOG(LogDBAUI, Log, TEXT("[LoginWidget] Native fallback layout created"));
}

void UDBALoginFlowWidgetBase::BindControls()
{
	if (LoginButton)
	{
		LoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleLoginClicked);
		LoginButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleLoginClicked);
	}
	if (GuestLoginButton)
	{
		GuestLoginButton->OnClicked.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleGuestLoginClicked);
		GuestLoginButton->OnClicked.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleGuestLoginClicked);
	}
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
}

void UDBALoginFlowWidgetBase::SetStatus(const FText& InStatusText)
{
	if (StatusText)
	{
		StatusText->SetText(InStatusText);
		StatusText->SetVisibility(InStatusText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

UDBALoginFlowSubsystem* UDBALoginFlowWidgetBase::GetLoginFlow() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>() : nullptr;
}

void UDBALoginFlowWidgetBase::HandleBackgroundMusicFinished()
{
	StartBackgroundMusic();
}

void UDBALoginFlowWidgetBase::InitializeAudioAssets()
{
	if (!ButtonClickSound)
	{
		ButtonClickSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick.SFX_UI_ButtonClick"));
		if (!ButtonClickSound)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] Button click sound not found."));
		}
	}

	if (!BackgroundMusicSound)
	{
		BackgroundMusicSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/DBA/Audio/UI/BGM/BGM_LoginFlow_Loop.BGM_LoginFlow_Loop"));
		if (!BackgroundMusicSound)
		{
			BackgroundMusicSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/DBA/Audio/UI/BGM/BGM_Login_Loop.BGM_Login_Loop"));
		}
		if (!BackgroundMusicSound)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] BGM asset not found."));
		}
	}
}

void UDBALoginFlowWidgetBase::StartBackgroundMusic()
{
	if (BackgroundMusicComponent || !BackgroundMusicSound || !GetWorld())
	{
		return;
	}

	BackgroundMusicComponent = UGameplayStatics::SpawnSound2D(GetWorld(), BackgroundMusicSound, 0.45f, 1.0f, 0.0f, nullptr, false, false);
	if (BackgroundMusicComponent)
	{
		BackgroundMusicComponent->bIsUISound = true;
		BackgroundMusicComponent->SetVolumeMultiplier(0.8f);
		BackgroundMusicComponent->OnAudioFinished.RemoveDynamic(this, &UDBALoginFlowWidgetBase::HandleBackgroundMusicFinished);
		BackgroundMusicComponent->OnAudioFinished.AddDynamic(this, &UDBALoginFlowWidgetBase::HandleBackgroundMusicFinished);
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[LoginWidget] Failed to spawn BGM component."));
	}
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
		LoginPanelTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginPanel_StoneGold.T_DBA_LoginPanel_StoneGold"));
	}

	if (!LoginButtonTexture)
	{
		LoginButtonTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginButton_ParchmentGold.T_DBA_LoginButton_ParchmentGold"));
	}

	if (!LoginBackgroundTexture)
	{
		LoginBackgroundTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginForestSanctuary.T_DBA_LoginForestSanctuary"));
	}
}

void UDBALoginFlowWidgetBase::BuildReferenceNativeLayout()
{
	if (!WidgetTree)
	{
		return;
	}

	const FDBALoginVisualLayoutSpec Spec = GetReferenceVisualLayoutSpec();

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ReferenceLoginRoot"));
	WidgetTree->RootWidget = RootCanvas;

	UImage* BackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ReferenceForestBackground"));
	if (LoginBackgroundTexture)
	{
		BackgroundImage->SetBrushFromTexture(LoginBackgroundTexture, true);
		BackgroundImage->SetColorAndOpacity(FLinearColor::White);
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

	UVerticalBox* LeftTools = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ReferenceLeftTools"));
	AddCanvasChild(RootCanvas, LeftTools, FVector2D(56.0f, 56.0f), FVector2D(104.0f, 330.0f));
	for (int32 Index = 0; Index < Spec.LeftToolLabels.Num(); ++Index)
	{
		const FText ButtonLabel = FText::Format(NSLOCTEXT("DBALoginFlowWidget", "LeftToolButtonFormat", "{0}\n{1}"),
			Index == 0 ? FText::FromString(TEXT("!")) : (Index == 1 ? FText::FromString(TEXT("*")) : FText::FromString(TEXT("?"))),
			Spec.LeftToolLabels[Index]);
		UButton* ToolButton = MakeTextButton(WidgetTree, FName(*FString::Printf(TEXT("ReferenceLeftTool_%d"), Index)), ButtonLabel, 24.0f, GoldText);
		AddVerticalChild(LeftTools, ToolButton, FMargin(0.0f, 0.0f, 0.0f, 24.0f));
	}

	TitleText = MakeText(WidgetTree, TEXT("ReferenceLoginTitle"), Spec.TitleText, 82.0f, FLinearColor(1.0f, 0.78f, 0.34f, 1.0f));
	AddCanvasChildAnchored(
		RootCanvas,
		TitleText,
		FAnchors(0.84f, 0.0f, 0.84f, 0.0f),
		FMargin(0.0f, 76.0f, 560.0f, 112.0f),
		FVector2D(0.5f, 0.0f));

	UTextBlock* SubtitleText = MakeText(WidgetTree, TEXT("ReferenceLoginSubtitle"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceSubtitle", "Divine Beasts Arena"), 30.0f, FLinearColor(1.0f, 0.80f, 0.45f, 1.0f));
	AddCanvasChildAnchored(
		RootCanvas,
		SubtitleText,
		FAnchors(0.84f, 0.0f, 0.84f, 0.0f),
		FMargin(0.0f, 178.0f, 540.0f, 48.0f),
		FVector2D(0.5f, 0.0f));

	UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LoginPanel"));
	LoginPanel = PanelBorder;
	AddCanvasChildAnchored(
		RootCanvas,
		PanelBorder,
		FAnchors(0.84f, 0.0f, 0.84f, 0.0f),
		FMargin(0.0f, 306.0f, 600.0f, 448.0f),
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
	PanelBorder->SetPadding(FMargin(48.0f, 54.0f, 48.0f, 40.0f));

	UVerticalBox* FormBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ReferenceLoginForm"));
	PanelBorder->SetContent(FormBox);

	UButton* ServerButton = MakeTextButton(WidgetTree, TEXT("ReferenceServerButton"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceServer", "推荐服务器 · 青木幻林        v"), 25.0f, GoldText);
	AddVerticalChild(FormBox, ServerButton, FMargin(0.0f, 0.0f, 0.0f, 18.0f));

	EmailInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("EmailInput"));
	EmailInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "AccountHint", "请输入账号"));
	ApplyEditableBoxStyle(EmailInput);
	AddVerticalChild(FormBox, EmailInput, FMargin(0.0f, 0.0f, 0.0f, 16.0f));

	PasswordInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("PasswordInput"));
	PasswordInput->SetHintText(NSLOCTEXT("DBALoginFlowWidget", "ReferencePasswordHint", "请输入密码"));
	PasswordInput->SetIsPassword(true);
	ApplyEditableBoxStyle(PasswordInput);
	AddVerticalChild(FormBox, PasswordInput, FMargin(0.0f, 0.0f, 0.0f, 12.0f));

	UHorizontalBox* OptionRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ReferenceLoginOptions"));
	AddVerticalChild(FormBox, OptionRow, FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	UTextBlock* RememberText = MakeText(WidgetTree, TEXT("ReferenceRememberText"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceRemember", "[x] 记住账号"), 22.0f, GoldText, ETextJustify::Left);
	if (UHorizontalBoxSlot* RememberSlot = OptionRow->AddChildToHorizontalBox(RememberText))
	{
		RememberSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	UTextBlock* ForgotText = MakeText(WidgetTree, TEXT("ReferenceForgotText"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceForgot", "忘记密码？"), 22.0f, GoldText, ETextJustify::Right);
	OptionRow->AddChildToHorizontalBox(ForgotText);

	LoginButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LoginButton"));
	LoginButton->AddChild(MakeLoginButtonLabel(WidgetTree, Spec.PrimaryButtonText));
	AddVerticalChild(FormBox, LoginButton, FMargin(0.0f), HAlign_Fill);

	ErrorText = MakeText(WidgetTree, TEXT("ErrorText"), FText::GetEmpty(), 18.0f, FLinearColor(1.0f, 0.35f, 0.24f, 1.0f));
	ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	AddVerticalChild(FormBox, ErrorText, FMargin(0.0f, 10.0f, 0.0f, 0.0f));

	StatusText = MakeText(WidgetTree, TEXT("StatusText"), FText::GetEmpty(), 18.0f, FLinearColor(0.76f, 0.96f, 0.74f, 1.0f));
	StatusText->SetVisibility(ESlateVisibility::Collapsed);
	AddVerticalChild(FormBox, StatusText, FMargin(0.0f, 6.0f, 0.0f, 0.0f));

	UTextBlock* OtherLoginText = MakeText(WidgetTree, TEXT("ReferenceOtherLoginText"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceOtherLogin", "其他登录方式"), 22.0f, GoldText);
	AddCanvasChildAnchored(
		RootCanvas,
		OtherLoginText,
		FAnchors(0.84f, 0.0f, 0.84f, 0.0f),
		FMargin(0.0f, 775.0f, 380.0f, 38.0f),
		FVector2D(0.5f, 0.0f));

	UHorizontalBox* OtherLoginRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ReferenceOtherLoginRow"));
	AddCanvasChildAnchored(
		RootCanvas,
		OtherLoginRow,
		FAnchors(0.84f, 0.0f, 0.84f, 0.0f),
		FMargin(0.0f, 822.0f, 512.0f, 96.0f),
		FVector2D(0.5f, 0.0f));
	GuestLoginButton = MakeTextButton(WidgetTree, TEXT("GuestLoginButton"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceGuestLogin", "神兽通行证"), 20.0f, GoldText);
	if (UHorizontalBoxSlot* GuestSlot = OtherLoginRow->AddChildToHorizontalBox(GuestLoginButton))
	{
		GuestSlot->SetPadding(FMargin(0.0f, 0.0f, 34.0f, 0.0f));
	}
	OtherLoginRow->AddChildToHorizontalBox(MakeTextButton(WidgetTree, TEXT("ReferenceSpiritLogin"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceSpiritLogin", "灵语"), 20.0f, GoldText));
	OtherLoginRow->AddChildToHorizontalBox(MakeTextButton(WidgetTree, TEXT("ReferenceJadeLogin"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceJadeLogin", "玉简登录"), 20.0f, GoldText));

	UButton* SwitchAccountButton = MakeTextButton(WidgetTree, TEXT("ReferenceSwitchAccount"), NSLOCTEXT("DBALoginFlowWidget", "ReferenceSwitchAccount", "切换账号"), 20.0f, GoldText);
	AddCanvasChildAnchored(
		RootCanvas,
		SwitchAccountButton,
		FAnchors(1.0f, 1.0f, 1.0f, 1.0f),
		FMargin(-220.0f, -72.0f, 190.0f, 48.0f));
}

void UDBALoginFlowWidgetBase::ApplyVisualStyle()
{
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
	ApplyButtonTextureStyle(GuestLoginButton);
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
