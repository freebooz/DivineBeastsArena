// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/AudioComponent.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/DBAUIFontUtils.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace
{
	UTextBlock* MakeLoginButtonLabel(UWidgetTree* WidgetTree, const FText& Label)
	{
		UTextBlock* TextBlock = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()) : nullptr;
		if (TextBlock)
		{
			TextBlock->SetText(Label);
		}
		return TextBlock;
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

void UDBALoginFlowWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBALoginFlowWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureNativeFallbackLayout();
	BindControls();
	InitializeAudioAssets();
	InitializeVisualAssets();
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

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NativeLoginTitle"));
	TitleText->SetText(NSLOCTEXT("DBALoginFlowWidget", "Title", "Divine Beasts Arena"));
	RootBox->AddChildToVerticalBox(TitleText);

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
		BackgroundMusicSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/DBA/Audio/UI/BGM/BGM_Login_Loop.BGM_Login_Loop"));
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
