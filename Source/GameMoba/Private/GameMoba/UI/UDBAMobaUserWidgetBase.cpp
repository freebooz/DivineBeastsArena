// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"

UDBAMobaUserWidgetBase::UDBAMobaUserWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<USoundBase> ClickSoundFinder(TEXT("/Game/DBA/Audio/SFX/Common/Impact/SFX_Impact_Generic_Hit.SFX_Impact_Generic_Hit"));
	if (ClickSoundFinder.Succeeded())
	{
		DefaultClickSound = ClickSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> BackgroundTextureFinder(TEXT("/Engine/EngineResources/Black.Black"));
	if (BackgroundTextureFinder.Succeeded())
	{
		DefaultBackgroundTexture = BackgroundTextureFinder.Object;
	}
}

void UDBAMobaUserWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (bAutoInjectBackground)
	{
		ApplyDefaultBackgroundTexture();
	}
}

void UDBAMobaUserWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	OwnerPlayerController = GetOwningPlayer();

	if (bAutoBindClickSound)
	{
		BindButtonClickAudio();
	}
}

void UDBAMobaUserWidgetBase::NativeDestruct()
{
	BoundButtons.Reset();
	InjectedBackgroundImage = nullptr;
	OwnerPlayerController.Reset();
	Super::NativeDestruct();
}

void UDBAMobaUserWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UDBAMobaUserWidgetBase::HandleAnyButtonClicked()
{
	USoundBase* ClickSound = DefaultClickSound.LoadSynchronous();
	if (!ClickSound)
	{
		return;
	}

	UGameplayStatics::PlaySound2D(this, ClickSound);
}

void UDBAMobaUserWidgetBase::BindButtonClickAudio()
{
	if (!WidgetTree)
	{
		return;
	}

	TArray<UWidget*> AllWidgets;
	WidgetTree->GetAllWidgets(AllWidgets);
	for (UWidget* Widget : AllWidgets)
	{
		UButton* Button = Cast<UButton>(Widget);
		if (!Button)
		{
			continue;
		}

		if (!BoundButtons.Contains(Button))
		{
			Button->OnClicked.AddDynamic(this, &UDBAMobaUserWidgetBase::HandleAnyButtonClicked);
			BoundButtons.Add(Button);
		}
	}
}

void UDBAMobaUserWidgetBase::ApplyDefaultBackgroundTexture()
{
	if (!WidgetTree || InjectedBackgroundImage)
	{
		return;
	}

	UTexture2D* Texture = DefaultBackgroundTexture.LoadSynchronous();
	if (!Texture)
	{
		return;
	}

	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return;
	}

	UImage* BackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("AutoBackgroundImage"));
	if (!BackgroundImage)
	{
		return;
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(Texture);
	BackgroundImage->SetBrush(Brush);
	BackgroundImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, BackgroundOpacity));

	UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(BackgroundImage);
	if (CanvasSlot)
	{
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		CanvasSlot->SetOffsets(FMargin(0.0f));
		CanvasSlot->SetZOrder(-1000);
	}

	InjectedBackgroundImage = BackgroundImage;
}
