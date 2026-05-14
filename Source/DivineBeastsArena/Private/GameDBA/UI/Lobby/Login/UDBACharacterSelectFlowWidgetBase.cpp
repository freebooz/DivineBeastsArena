// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/AudioComponent.h"
#include "Components/Border.h"
#include "Components/Button.h"
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
#include "EngineUtils.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
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
		return LowerName.Contains(TEXT("background"))
			|| LowerName.Contains(TEXT("backdrop"))
			|| LowerName.StartsWith(TEXT("bg"))
			|| LowerName.Contains(TEXT("_bg"));
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

	ADBACharacterPresentationActor* ResolveSelectPresentationActor(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		for (TActorIterator<ADBACharacterPresentationActor> It(World); It; ++It)
		{
			if (IsValid(*It))
			{
				return *It;
			}
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("DBA_CharacterPresentationStage");
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<ADBACharacterPresentationActor>(
			ADBACharacterPresentationActor::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);
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

FReply UDBACharacterSelectFlowWidgetBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && IsPointerOverPreviewHost(InMouseEvent.GetScreenSpacePosition()))
	{
		BeginPreviewRotationDrag(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled().CaptureMouse(TakeWidget());
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
		UpdatePreviewRotationDrag(InMouseEvent.GetScreenSpacePosition());
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
	if (!WidgetTree || WidgetTree->RootWidget || (CharacterListText && ConfirmButton))
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

	PreviewActor = ResolveSelectPresentationActor(GetWorld());
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
