// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/AudioComponent.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ContentWidget.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Viewport.h"
#include "Components/VerticalBox.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPreviewActor.h"
#include "GameFramework/PlayerController.h"
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

		return WidgetTree->RootWidget;
	}

	bool TryAttachSelectPreviewViewport(UWidget* HostWidget, UViewport* Viewport)
	{
		if (!HostWidget || !Viewport)
		{
			return false;
		}

		if (UPanelWidget* PanelHost = Cast<UPanelWidget>(HostWidget))
		{
			PanelHost->AddChild(Viewport);
			return true;
		}

		if (UContentWidget* ContentHost = Cast<UContentWidget>(HostWidget))
		{
			ContentHost->SetContent(Viewport);
			return true;
		}

		return false;
	}

	void ApplySelectHostTransparency(UWidget* HostWidget)
	{
		if (UBorder* Border = Cast<UBorder>(HostWidget))
		{
			FLinearColor Color = Border->GetBrushColor();
			Color.A = 0.0f;
			Border->SetBrushColor(Color);
		}
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
		PC->SetShowMouseCursor(true);
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		Widget->SetFocus();
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
	InitializeAudioAssets();
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
	ConfirmButton->AddChild(MakeSelectButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterSelectWidget", "ConfirmButton", "Enter Lobby")));
	RootBox->AddChildToVerticalBox(ConfirmButton);

	CreateButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CreateButton"));
	CreateButton->AddChild(MakeSelectButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterSelectWidget", "CreateButton", "Create Character")));
	RootBox->AddChildToVerticalBox(CreateButton);

	RefreshButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RefreshButton"));
	RefreshButton->AddChild(MakeSelectButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterSelectWidget", "RefreshButton", "Refresh")));
	RootBox->AddChildToVerticalBox(RefreshButton);

	CharacterPreviewViewport = WidgetTree->ConstructWidget<UViewport>(UViewport::StaticClass(), TEXT("CharacterPreviewViewport"));
	RootBox->AddChildToVerticalBox(CharacterPreviewViewport);

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
	if (!CharacterPreviewViewport)
	{
		CharacterPreviewHost = ResolveSelectPreviewHost(WidgetTree, CharacterPreviewHost);
		ApplySelectHostTransparency(CharacterPreviewHost);
		if (CharacterPreviewHost && WidgetTree)
		{
			CharacterPreviewViewport = WidgetTree->ConstructWidget<UViewport>(UViewport::StaticClass(), TEXT("CharacterPreviewViewport_Auto"));
			if (CharacterPreviewViewport && TryAttachSelectPreviewViewport(CharacterPreviewHost, CharacterPreviewViewport))
			{
				UE_LOG(LogDBAUI, Warning, TEXT("[CharacterSelectWidget] Auto-created CharacterPreviewViewport under host '%s'."), *CharacterPreviewHost->GetName());
			}
		}
	}

	if (!CharacterPreviewViewport || PreviewActor)
	{
		return;
	}

	CharacterPreviewViewport->SetViewLocation(FVector(260.0f, 0.0f, 100.0f));
	CharacterPreviewViewport->SetViewRotation(FRotator(-8.0f, 180.0f, 0.0f));
	CharacterPreviewViewport->SetEnableAdvancedFeatures(false);
	CharacterPreviewViewport->SetBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));

	PreviewActor = Cast<ADBACharacterPreviewActor>(CharacterPreviewViewport->Spawn(ADBACharacterPreviewActor::StaticClass()));
	if (PreviewActor)
	{
		PreviewActor->SetActorLocation(FVector::ZeroVector);
		PreviewActor->SetRotationSpeed(8.0f);
	}

	PreviewDirectionalLight = Cast<ADirectionalLight>(CharacterPreviewViewport->Spawn(ADirectionalLight::StaticClass()));
	if (PreviewDirectionalLight && PreviewDirectionalLight->GetLightComponent())
	{
		PreviewDirectionalLight->SetActorRotation(FRotator(-32.0f, -28.0f, 0.0f));
		PreviewDirectionalLight->GetLightComponent()->SetCastShadows(false);
		PreviewDirectionalLight->GetLightComponent()->SetIntensity(1650.0f);
		PreviewDirectionalLight->GetLightComponent()->SetLightColor(FLinearColor(1.0f, 0.88f, 0.68f));
	}

	PreviewFillLight = Cast<ADirectionalLight>(CharacterPreviewViewport->Spawn(ADirectionalLight::StaticClass()));
	if (PreviewFillLight && PreviewFillLight->GetLightComponent())
	{
		PreviewFillLight->SetActorRotation(FRotator(-8.0f, 145.0f, 0.0f));
		PreviewFillLight->GetLightComponent()->SetCastShadows(false);
		PreviewFillLight->GetLightComponent()->SetIntensity(520.0f);
		PreviewFillLight->GetLightComponent()->SetLightColor(FLinearColor(0.62f, 0.76f, 1.0f));
	}

	PreviewSkyLight = Cast<ASkyLight>(CharacterPreviewViewport->Spawn(ASkyLight::StaticClass()));
	if (PreviewSkyLight && PreviewSkyLight->GetLightComponent())
	{
		PreviewSkyLight->GetLightComponent()->SetCastShadows(false);
		PreviewSkyLight->GetLightComponent()->SetIntensity(1.35f);
		PreviewSkyLight->GetLightComponent()->SetLightColor(FLinearColor(0.72f, 0.80f, 1.0f));
	}
}

void UDBACharacterSelectFlowWidgetBase::DestroyPreviewViewport()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
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
