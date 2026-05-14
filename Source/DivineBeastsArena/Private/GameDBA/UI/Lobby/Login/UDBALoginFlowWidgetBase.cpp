// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "GameDBA/Core/DBALogChannels.h"

namespace
{
	UTextBlock* MakeButtonLabel(UWidgetTree* WidgetTree, const FText& Label)
	{
		UTextBlock* TextBlock = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()) : nullptr;
		if (TextBlock)
		{
			TextBlock->SetText(Label);
		}
		return TextBlock;
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
	ClearError();

	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
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
	SubmitLogin();
}

void UDBALoginFlowWidgetBase::HandleGuestLoginClicked()
{
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

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NativeLoginRoot"));
	WidgetTree->RootWidget = RootBox;

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
	LoginButton->AddChild(MakeButtonLabel(WidgetTree, NSLOCTEXT("DBALoginFlowWidget", "LoginButton", "Login")));
	RootBox->AddChildToVerticalBox(LoginButton);

	GuestLoginButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("GuestLoginButton"));
	GuestLoginButton->AddChild(MakeButtonLabel(WidgetTree, NSLOCTEXT("DBALoginFlowWidget", "GuestButton", "Guest Login")));
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
