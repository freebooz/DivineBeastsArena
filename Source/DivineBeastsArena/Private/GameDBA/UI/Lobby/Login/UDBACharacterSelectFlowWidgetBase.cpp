// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Viewport.h"
#include "Components/VerticalBox.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPreviewActor.h"

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
}

UDBACharacterSelectFlowWidgetBase::UDBACharacterSelectFlowWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACharacterSelectFlowWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBACharacterSelectFlowWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureNativeFallbackLayout();
	BindControls();
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
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "NoSelection", "No character selected."));
		return;
	}

	if (UDBALoginFlowSubsystem* LoginFlow = GetLoginFlow())
	{
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "EnteringLobby", "Entering lobby..."));
		LoginFlow->SubmitCharacterSelection(SelectedCharacterId);
	}
	else
	{
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "FlowUnavailable", "Login flow unavailable."));
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
		SetStatus(NSLOCTEXT("DBACharacterSelectWidget", "Refreshing", "Refreshing character list..."));
		LoginFlow->RefreshCharacterList();
	}
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
	if (!WidgetTree || (CharacterListText && ConfirmButton))
	{
		return;
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NativeCharacterSelectRoot"));
	WidgetTree->RootWidget = RootBox;

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NativeSelectTitle"));
	TitleText->SetText(NSLOCTEXT("DBACharacterSelectWidget", "Title", "Select Character"));
	RootBox->AddChildToVerticalBox(TitleText);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	RootBox->AddChildToVerticalBox(StatusText);

	CharacterListText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CharacterListText"));
	RootBox->AddChildToVerticalBox(CharacterListText);

	ConfirmButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ConfirmButton"));
	ConfirmButton->AddChild(MakeButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterSelectWidget", "ConfirmButton", "Enter Lobby")));
	RootBox->AddChildToVerticalBox(ConfirmButton);

	CreateButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CreateButton"));
	CreateButton->AddChild(MakeButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterSelectWidget", "CreateButton", "Create Character")));
	RootBox->AddChildToVerticalBox(CreateButton);

	RefreshButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RefreshButton"));
	RefreshButton->AddChild(MakeButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterSelectWidget", "RefreshButton", "Refresh")));
	RootBox->AddChildToVerticalBox(RefreshButton);

	UE_LOG(LogDBAUI, Log, TEXT("[CharacterSelectWidget] Native fallback layout created"));
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
		CharacterListText->SetText(NSLOCTEXT("DBACharacterSelectWidget", "NoCharacters", "No characters found."));
		return;
	}

	TArray<FString> Lines;
	for (const FDBACharacterSummary& Character : CurrentCharacters)
	{
		const FString Prefix = Character.CharacterId == SelectedCharacterId ? TEXT("> ") : TEXT("  ");
		Lines.Add(FString::Printf(TEXT("%s%s  Lv.%d"), *Prefix, *Character.CharacterName, Character.Level));
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
	if (!CharacterPreviewViewport || PreviewActor)
	{
		return;
	}

	CharacterPreviewViewport->SetViewLocation(FVector(120.0f, 0.0f, 80.0f));
	CharacterPreviewViewport->SetViewRotation(FRotator(0.0f, 180.0f, 0.0f));

	PreviewActor = Cast<ADBACharacterPreviewActor>(CharacterPreviewViewport->Spawn(ADBACharacterPreviewActor::StaticClass()));
	if (PreviewActor)
	{
		PreviewActor->SetRotationSpeed(15.0f);
	}
}

void UDBACharacterSelectFlowWidgetBase::DestroyPreviewViewport()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
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
