// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Viewport.h"
#include "Components/VerticalBox.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPreviewActor.h"

namespace
{
	template <typename EnumType>
	FText EnumValueText(EnumType Value)
	{
		if (const UEnum* Enum = StaticEnum<EnumType>())
		{
			return Enum->GetDisplayNameTextByValue(static_cast<int64>(Value));
		}
		return FText::FromString(TEXT("Unknown"));
	}

	UTextBlock* MakeButtonLabel(UWidgetTree* WidgetTree, const FText& Label)
	{
		UTextBlock* TextBlock = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()) : nullptr;
		if (TextBlock)
		{
			TextBlock->SetText(Label);
		}
		return TextBlock;
	}

	template <typename EnumType>
	EnumType CycleEnumValue(EnumType Current, const TArray<EnumType>& Values)
	{
		const int32 CurrentIndex = Values.IndexOfByKey(Current);
		return Values.IsValidIndex(CurrentIndex + 1) ? Values[CurrentIndex + 1] : Values[0];
	}
}

UDBACharacterCreateFlowWidgetBase::UDBACharacterCreateFlowWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACharacterCreateFlowWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBACharacterCreateFlowWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureNativeFallbackLayout();
	BindControls();
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
		ShowValidationMessage(true, NSLOCTEXT("DBACharacterCreateWidget", "Creating", "Creating character..."));
		LoginFlow->SubmitCharacterCreation(Request);
		UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] Submitted character creation: %s"), *CharacterName);
	}
	else
	{
		ShowValidationMessage(false, NSLOCTEXT("DBACharacterCreateWidget", "FlowUnavailable", "Login flow unavailable."));
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
	if (!WidgetTree || (CharacterNameInput && CreateButton))
	{
		return;
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NativeCharacterCreateRoot"));
	WidgetTree->RootWidget = RootBox;

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NativeCreateTitle"));
	TitleText->SetText(NSLOCTEXT("DBACharacterCreateWidget", "Title", "Create Character"));
	RootBox->AddChildToVerticalBox(TitleText);

	ValidationText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ValidationText"));
	RootBox->AddChildToVerticalBox(ValidationText);

	CharacterNameInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("CharacterNameInput"));
	CharacterNameInput->SetHintText(NSLOCTEXT("DBACharacterCreateWidget", "NameHint", "Character name"));
	RootBox->AddChildToVerticalBox(CharacterNameInput);

	ZodiacButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ZodiacButton"));
	ZodiacText = MakeButtonLabel(WidgetTree, FText::GetEmpty());
	ZodiacButton->AddChild(ZodiacText);
	RootBox->AddChildToVerticalBox(ZodiacButton);

	ElementButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ElementButton"));
	ElementText = MakeButtonLabel(WidgetTree, FText::GetEmpty());
	ElementButton->AddChild(ElementText);
	RootBox->AddChildToVerticalBox(ElementButton);

	FiveCampButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("FiveCampButton"));
	FiveCampText = MakeButtonLabel(WidgetTree, FText::GetEmpty());
	FiveCampButton->AddChild(FiveCampText);
	RootBox->AddChildToVerticalBox(FiveCampButton);

	CreateButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CreateButton"));
	CreateButton->AddChild(MakeButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterCreateWidget", "CreateButton", "Create and Enter")));
	RootBox->AddChildToVerticalBox(CreateButton);

	BackButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BackButton"));
	BackButton->AddChild(MakeButtonLabel(WidgetTree, NSLOCTEXT("DBACharacterCreateWidget", "BackButton", "Back to Character Select")));
	RootBox->AddChildToVerticalBox(BackButton);

	UE_LOG(LogDBAUI, Log, TEXT("[CharacterCreateWidget] Native fallback layout created"));
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

	bIsCreateValid = !CharacterName.IsEmpty() && SelectedZodiac != EDBAZodiac::None && SelectedElement != EDBAElement::None;
	ShowValidationMessage(
		bIsCreateValid,
		bIsCreateValid
			? NSLOCTEXT("DBACharacterCreateWidget", "Ready", "Ready.")
			: NSLOCTEXT("DBACharacterCreateWidget", "MissingName", "Enter a character name."));
	return bIsCreateValid;
}

void UDBACharacterCreateFlowWidgetBase::RefreshChoiceText()
{
	if (ZodiacText)
	{
		ZodiacText->SetText(FText::Format(NSLOCTEXT("DBACharacterCreateWidget", "ZodiacFormat", "Zodiac: {0}"), EnumValueText(SelectedZodiac)));
	}
	if (ElementText)
	{
		ElementText->SetText(FText::Format(NSLOCTEXT("DBACharacterCreateWidget", "ElementFormat", "Element: {0}"), EnumValueText(SelectedElement)));
	}
	if (FiveCampText)
	{
		FiveCampText->SetText(FText::Format(NSLOCTEXT("DBACharacterCreateWidget", "CampFormat", "Camp: {0}"), EnumValueText(SelectedFiveCamp)));
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
	if (!CharacterPreviewViewport || PreviewActor)
	{
		return;
	}

	CharacterPreviewViewport->SetViewLocation(FVector(120.0f, 0.0f, 80.0f));
	CharacterPreviewViewport->SetViewRotation(FRotator(0.0f, 180.0f, 0.0f));

	PreviewActor = Cast<ADBACharacterPreviewActor>(CharacterPreviewViewport->Spawn(ADBACharacterPreviewActor::StaticClass()));
	if (PreviewActor)
	{
		PreviewActor->SetRotationSpeed(12.0f);
		PreviewActor->SetPreviewZodiac(SelectedZodiac);
	}
}

void UDBACharacterCreateFlowWidgetBase::DestroyPreviewViewport()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

void UDBACharacterCreateFlowWidgetBase::RefreshPreviewCharacter()
{
	if (PreviewActor)
	{
		PreviewActor->SetPreviewZodiac(SelectedZodiac);
	}
}
