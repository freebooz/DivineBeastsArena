// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/UDBAGameSettingsWidgetBase.h"

#include "AudioDevice.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/Texture2D.h"
#include "GameDBA/Player/DBALobbyPlayerController.h"
#include "GameDBA/UI/DBAGameUIManager.h"
#include "GameFramework/GameUserSettings.h"
#include "InputCoreTypes.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	float GDBACachedMasterVolume = 1.0f;

	UWidget* FindSettingsWidgetByNames(UWidgetTree* WidgetTree, const TArray<FName>& Names)
	{
		if (!WidgetTree)
		{
			return nullptr;
		}

		for (const FName& Name : Names)
		{
			if (UWidget* Widget = WidgetTree->FindWidget(Name))
			{
				return Widget;
			}
		}
		return nullptr;
	}

	UTextBlock* MakeSettingsLabel(UWidgetTree* WidgetTree, const FText& Text)
	{
		UTextBlock* Label = WidgetTree ? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()) : nullptr;
		if (Label)
		{
			Label->SetText(Text);
		}
		return Label;
	}

	void AddSettingsButtonText(UWidgetTree* WidgetTree, UButton* Button, const FText& Text)
	{
		if (!WidgetTree || !Button)
		{
			return;
		}

		Button->AddChild(MakeSettingsLabel(WidgetTree, Text));
	}

	USlider* AddSettingsSliderRow(UWidgetTree* WidgetTree, UVerticalBox* RootBox, const TCHAR* WidgetName, const FText& LabelText)
	{
		if (!WidgetTree || !RootBox)
		{
			return nullptr;
		}

		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		RootBox->AddChildToVerticalBox(Row);
		Row->AddChildToHorizontalBox(MakeSettingsLabel(WidgetTree, LabelText));

		USlider* Slider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), WidgetName);
		Slider->SetMinValue(0.0f);
		Slider->SetMaxValue(1.0f);
		Slider->SetStepSize(0.01f);
		Row->AddChildToHorizontalBox(Slider);
		return Slider;
	}
}

UDBAGameSettingsWidgetBase::UDBAGameSettingsWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	bAutoInjectBackground = false;
	bAutoBindClickSound = false;
	PanelBackgroundTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginPanel_StoneGold.T_DBA_LoginPanel_StoneGold")));
	CurrentMasterVolume = GDBACachedMasterVolume;
}

void UDBAGameSettingsWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureNativeFallbackLayout();
	ResolveBoundWidgetsFromWidgetTree();
	ApplyPanelBackgroundTexture();
	BindControls();
	RefreshFromRuntime();
	SetKeyboardFocus();
}

void UDBAGameSettingsWidgetBase::NativeDestruct()
{
	UnbindControls();
	Super::NativeDestruct();
}

FReply UDBAGameSettingsWidgetBase::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseSettings();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UDBAGameSettingsWidgetBase::RefreshFromRuntime()
{
	CurrentMasterVolume = GDBACachedMasterVolume;

	if (ADBALobbyPlayerController* LobbyPC = Cast<ADBALobbyPlayerController>(GetOwningPlayer()))
	{
		CurrentMouseSensitivity = LobbyPC->GetMouseLookSensitivityValue();
		CurrentCameraDistance = LobbyPC->GetCameraDistanceValue();
	}
	else
	{
		CurrentMouseSensitivity = DefaultMouseSensitivity;
		CurrentCameraDistance = DefaultCameraDistance;
	}

	if (const UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		bCurrentFullscreen = UserSettings->GetFullscreenMode() != EWindowMode::Windowed;
	}

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetValue(FMath::Clamp(CurrentMasterVolume, 0.0f, 1.0f));
	}
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->SetValue(MouseSensitivityToSlider(CurrentMouseSensitivity));
	}
	if (CameraDistanceSlider)
	{
		CameraDistanceSlider->SetValue(CameraDistanceToSlider(CurrentCameraDistance));
	}
	if (FullscreenCheckBox)
	{
		FullscreenCheckBox->SetIsChecked(bCurrentFullscreen);
	}

	UpdateStatus(NSLOCTEXT("DBAGameSettings", "Ready", "设置已就绪"));
}

void UDBAGameSettingsWidgetBase::ApplySettings()
{
	GDBACachedMasterVolume = FMath::Clamp(CurrentMasterVolume, 0.0f, 1.0f);
	ApplyMasterVolume(GDBACachedMasterVolume);

	if (ADBALobbyPlayerController* LobbyPC = Cast<ADBALobbyPlayerController>(GetOwningPlayer()))
	{
		LobbyPC->SetMouseLookSensitivityValue(CurrentMouseSensitivity);
		LobbyPC->SetCameraDistanceValue(CurrentCameraDistance);
	}

	if (UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UserSettings->SetFullscreenMode(bCurrentFullscreen ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed);
		UserSettings->ApplySettings(false);
		UserSettings->SaveSettings();
	}

	BP_OnSettingsApplied(GDBACachedMasterVolume, CurrentMouseSensitivity, CurrentCameraDistance, bCurrentFullscreen);
	UpdateStatus(NSLOCTEXT("DBAGameSettings", "Applied", "设置已应用"));
}

void UDBAGameSettingsWidgetBase::ResetDefaults()
{
	CurrentMasterVolume = DefaultMasterVolume;
	CurrentMouseSensitivity = DefaultMouseSensitivity;
	CurrentCameraDistance = DefaultCameraDistance;
	bCurrentFullscreen = false;

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetValue(CurrentMasterVolume);
	}
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->SetValue(MouseSensitivityToSlider(CurrentMouseSensitivity));
	}
	if (CameraDistanceSlider)
	{
		CameraDistanceSlider->SetValue(CameraDistanceToSlider(CurrentCameraDistance));
	}
	if (FullscreenCheckBox)
	{
		FullscreenCheckBox->SetIsChecked(false);
	}

	ApplySettings();
}

void UDBAGameSettingsWidgetBase::CloseSettings()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->HideGameSettings();
			return;
		}
	}

	RemoveFromParent();
}

void UDBAGameSettingsWidgetBase::EnsureNativeFallbackLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("NativeGameSettingsCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	PanelBackgroundBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PanelBackgroundBorder"));
	PanelBackgroundBorder->SetPadding(FMargin(42.0f, 38.0f, 42.0f, 34.0f));
	PanelBackgroundBorder->SetBrushColor(FLinearColor(0.08f, 0.065f, 0.045f, 0.96f));

	if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelBackgroundBorder))
	{
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetOffsets(FMargin(0.0f, 0.0f, NativePanelSize.X, NativePanelSize.Y));
		PanelSlot->SetZOrder(10);
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NativeGameSettingsRoot"));
	PanelBackgroundBorder->SetContent(RootBox);

	RootBox->AddChildToVerticalBox(MakeSettingsLabel(WidgetTree, NSLOCTEXT("DBAGameSettings", "Title", "游戏设置")));
	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusText"));
	RootBox->AddChildToVerticalBox(StatusText);

	MasterVolumeSlider = AddSettingsSliderRow(WidgetTree, RootBox, TEXT("MasterVolumeSlider"), NSLOCTEXT("DBAGameSettings", "MasterVolume", "主音量"));
	MouseSensitivitySlider = AddSettingsSliderRow(WidgetTree, RootBox, TEXT("MouseSensitivitySlider"), NSLOCTEXT("DBAGameSettings", "MouseSensitivity", "鼠标灵敏度"));
	CameraDistanceSlider = AddSettingsSliderRow(WidgetTree, RootBox, TEXT("CameraDistanceSlider"), NSLOCTEXT("DBAGameSettings", "CameraDistance", "摄像机距离"));

	UHorizontalBox* FullscreenRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	RootBox->AddChildToVerticalBox(FullscreenRow);
	FullscreenRow->AddChildToHorizontalBox(MakeSettingsLabel(WidgetTree, NSLOCTEXT("DBAGameSettings", "Fullscreen", "无边框全屏")));
	FullscreenCheckBox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), TEXT("FullscreenCheckBox"));
	FullscreenRow->AddChildToHorizontalBox(FullscreenCheckBox);

	UHorizontalBox* ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	RootBox->AddChildToVerticalBox(ButtonRow);

	ApplyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ApplyButton"));
	AddSettingsButtonText(WidgetTree, ApplyButton, NSLOCTEXT("DBAGameSettings", "Apply", "应用"));
	ButtonRow->AddChildToHorizontalBox(ApplyButton);

	ResetButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ResetButton"));
	AddSettingsButtonText(WidgetTree, ResetButton, NSLOCTEXT("DBAGameSettings", "Reset", "重置"));
	ButtonRow->AddChildToHorizontalBox(ResetButton);

	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
	AddSettingsButtonText(WidgetTree, CloseButton, NSLOCTEXT("DBAGameSettings", "Close", "关闭"));
	ButtonRow->AddChildToHorizontalBox(CloseButton);
}

void UDBAGameSettingsWidgetBase::ResolveBoundWidgetsFromWidgetTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!CloseButton)
	{
		CloseButton = Cast<UButton>(FindSettingsWidgetByNames(WidgetTree, { TEXT("CloseButton"), TEXT("BackButton"), TEXT("ResumeButton") }));
	}
	if (!ApplyButton)
	{
		ApplyButton = Cast<UButton>(FindSettingsWidgetByNames(WidgetTree, { TEXT("ApplyButton"), TEXT("SaveButton") }));
	}
	if (!ResetButton)
	{
		ResetButton = Cast<UButton>(FindSettingsWidgetByNames(WidgetTree, { TEXT("ResetButton"), TEXT("DefaultsButton"), TEXT("ResetDefaultsButton") }));
	}
	if (!MasterVolumeSlider)
	{
		MasterVolumeSlider = Cast<USlider>(FindSettingsWidgetByNames(WidgetTree, { TEXT("MasterVolumeSlider"), TEXT("VolumeSlider") }));
	}
	if (!MouseSensitivitySlider)
	{
		MouseSensitivitySlider = Cast<USlider>(FindSettingsWidgetByNames(WidgetTree, { TEXT("MouseSensitivitySlider"), TEXT("SensitivitySlider") }));
	}
	if (!CameraDistanceSlider)
	{
		CameraDistanceSlider = Cast<USlider>(FindSettingsWidgetByNames(WidgetTree, { TEXT("CameraDistanceSlider"), TEXT("CameraZoomSlider") }));
	}
	if (!FullscreenCheckBox)
	{
		FullscreenCheckBox = Cast<UCheckBox>(FindSettingsWidgetByNames(WidgetTree, { TEXT("FullscreenCheckBox"), TEXT("FullscreenToggle") }));
	}
	if (!StatusText)
	{
		StatusText = Cast<UTextBlock>(FindSettingsWidgetByNames(WidgetTree, { TEXT("StatusText"), TEXT("SettingsStatusText") }));
	}
	if (!PanelBackgroundBorder)
	{
		PanelBackgroundBorder = Cast<UBorder>(FindSettingsWidgetByNames(WidgetTree, {
			TEXT("PanelBackgroundBorder"),
			TEXT("SettingsPanelBackground"),
			TEXT("SettingsPanel"),
			TEXT("RootPanel")
		}));
	}
	if (!PanelBackgroundImage)
	{
		PanelBackgroundImage = Cast<UImage>(FindSettingsWidgetByNames(WidgetTree, {
			TEXT("PanelBackgroundImage"),
			TEXT("SettingsPanelBackgroundImage"),
			TEXT("SettingsBackgroundImage")
		}));
	}
}

void UDBAGameSettingsWidgetBase::ApplyPanelBackgroundTexture()
{
	UTexture2D* Texture = PanelBackgroundTexture.LoadSynchronous();
	if (PanelBackgroundBorder)
	{
		if (Texture)
		{
			PanelBackgroundBorder->SetBrushFromTexture(Texture);
			PanelBackgroundBorder->SetBrushColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.96f));
		}
		else
		{
			PanelBackgroundBorder->SetBrushColor(FLinearColor(0.08f, 0.065f, 0.045f, 0.96f));
		}
	}
	if (PanelBackgroundImage)
	{
		if (Texture)
		{
			PanelBackgroundImage->SetBrushFromTexture(Texture, true);
			PanelBackgroundImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.96f));
		}
		else
		{
			PanelBackgroundImage->SetColorAndOpacity(FLinearColor(0.08f, 0.065f, 0.045f, 0.96f));
		}
	}
}

void UDBAGameSettingsWidgetBase::BindControls()
{
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleCloseClicked);
		CloseButton->OnClicked.AddDynamic(this, &UDBAGameSettingsWidgetBase::HandleCloseClicked);
	}
	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleApplyClicked);
		ApplyButton->OnClicked.AddDynamic(this, &UDBAGameSettingsWidgetBase::HandleApplyClicked);
	}
	if (ResetButton)
	{
		ResetButton->OnClicked.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleResetClicked);
		ResetButton->OnClicked.AddDynamic(this, &UDBAGameSettingsWidgetBase::HandleResetClicked);
	}
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleMasterVolumeChanged);
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UDBAGameSettingsWidgetBase::HandleMasterVolumeChanged);
	}
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleMouseSensitivityChanged);
		MouseSensitivitySlider->OnValueChanged.AddDynamic(this, &UDBAGameSettingsWidgetBase::HandleMouseSensitivityChanged);
	}
	if (CameraDistanceSlider)
	{
		CameraDistanceSlider->OnValueChanged.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleCameraDistanceChanged);
		CameraDistanceSlider->OnValueChanged.AddDynamic(this, &UDBAGameSettingsWidgetBase::HandleCameraDistanceChanged);
	}
	if (FullscreenCheckBox)
	{
		FullscreenCheckBox->OnCheckStateChanged.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleFullscreenChanged);
		FullscreenCheckBox->OnCheckStateChanged.AddDynamic(this, &UDBAGameSettingsWidgetBase::HandleFullscreenChanged);
	}
}

void UDBAGameSettingsWidgetBase::UnbindControls()
{
	if (CloseButton) { CloseButton->OnClicked.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleCloseClicked); }
	if (ApplyButton) { ApplyButton->OnClicked.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleApplyClicked); }
	if (ResetButton) { ResetButton->OnClicked.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleResetClicked); }
	if (MasterVolumeSlider) { MasterVolumeSlider->OnValueChanged.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleMasterVolumeChanged); }
	if (MouseSensitivitySlider) { MouseSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleMouseSensitivityChanged); }
	if (CameraDistanceSlider) { CameraDistanceSlider->OnValueChanged.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleCameraDistanceChanged); }
	if (FullscreenCheckBox) { FullscreenCheckBox->OnCheckStateChanged.RemoveDynamic(this, &UDBAGameSettingsWidgetBase::HandleFullscreenChanged); }
}

void UDBAGameSettingsWidgetBase::UpdateStatus(const FText& Text)
{
	if (StatusText)
	{
		StatusText->SetText(Text);
	}
}

void UDBAGameSettingsWidgetBase::ApplyMasterVolume(float Volume) const
{
	if (FAudioDevice* AudioDevice = GetWorld() ? GetWorld()->GetAudioDeviceRaw() : nullptr)
	{
		AudioDevice->SetTransientPrimaryVolume(FMath::Clamp(Volume, 0.0f, 1.0f));
	}
}

float UDBAGameSettingsWidgetBase::SliderToMouseSensitivity(float SliderValue) const
{
	return FMath::Lerp(MinMouseSensitivity, MaxMouseSensitivity, FMath::Clamp(SliderValue, 0.0f, 1.0f));
}

float UDBAGameSettingsWidgetBase::MouseSensitivityToSlider(float Sensitivity) const
{
	return FMath::GetMappedRangeValueClamped(FVector2D(MinMouseSensitivity, MaxMouseSensitivity), FVector2D(0.0f, 1.0f), Sensitivity);
}

float UDBAGameSettingsWidgetBase::SliderToCameraDistance(float SliderValue) const
{
	ADBALobbyPlayerController* LobbyPC = Cast<ADBALobbyPlayerController>(GetOwningPlayer());
	const float MinDistance = LobbyPC ? LobbyPC->GetMinCameraDistanceValue() : 280.0f;
	const float MaxDistance = LobbyPC ? LobbyPC->GetMaxCameraDistanceValue() : 950.0f;
	return FMath::Lerp(MinDistance, MaxDistance, FMath::Clamp(SliderValue, 0.0f, 1.0f));
}

float UDBAGameSettingsWidgetBase::CameraDistanceToSlider(float Distance) const
{
	ADBALobbyPlayerController* LobbyPC = Cast<ADBALobbyPlayerController>(GetOwningPlayer());
	const float MinDistance = LobbyPC ? LobbyPC->GetMinCameraDistanceValue() : 280.0f;
	const float MaxDistance = LobbyPC ? LobbyPC->GetMaxCameraDistanceValue() : 950.0f;
	return FMath::GetMappedRangeValueClamped(FVector2D(MinDistance, MaxDistance), FVector2D(0.0f, 1.0f), Distance);
}

void UDBAGameSettingsWidgetBase::HandleCloseClicked()
{
	CloseSettings();
}

void UDBAGameSettingsWidgetBase::HandleApplyClicked()
{
	ApplySettings();
}

void UDBAGameSettingsWidgetBase::HandleResetClicked()
{
	ResetDefaults();
}

void UDBAGameSettingsWidgetBase::HandleMasterVolumeChanged(float Value)
{
	CurrentMasterVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	ApplyMasterVolume(CurrentMasterVolume);
}

void UDBAGameSettingsWidgetBase::HandleMouseSensitivityChanged(float Value)
{
	CurrentMouseSensitivity = SliderToMouseSensitivity(Value);
	if (ADBALobbyPlayerController* LobbyPC = Cast<ADBALobbyPlayerController>(GetOwningPlayer()))
	{
		LobbyPC->SetMouseLookSensitivityValue(CurrentMouseSensitivity);
	}
}

void UDBAGameSettingsWidgetBase::HandleCameraDistanceChanged(float Value)
{
	CurrentCameraDistance = SliderToCameraDistance(Value);
	if (ADBALobbyPlayerController* LobbyPC = Cast<ADBALobbyPlayerController>(GetOwningPlayer()))
	{
		LobbyPC->SetCameraDistanceValue(CurrentCameraDistance);
	}
}

void UDBAGameSettingsWidgetBase::HandleFullscreenChanged(bool bIsChecked)
{
	bCurrentFullscreen = bIsChecked;
}
