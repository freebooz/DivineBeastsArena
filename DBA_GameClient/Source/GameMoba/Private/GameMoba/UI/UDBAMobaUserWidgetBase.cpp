// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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
#include "UObject/SoftObjectPath.h"
#include "GameFramework/PlayerController.h"

UDBAMobaUserWidgetBase::UDBAMobaUserWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultClickSound = TSoftObjectPtr<USoundBase>(
		FSoftObjectPath(TEXT("/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick.SFX_UI_ButtonClick")));

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
	for (UButton* Button : BoundButtons)
	{
		if (Button)
		{
			Button->OnClicked.RemoveDynamic(this, &UDBAMobaUserWidgetBase::HandleAnyButtonClicked);
		}
	}
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

	UGameplayStatics::PlaySound2D(this, ClickSound, 0.85f, 1.0f, 0.0f, nullptr, nullptr, true);
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
			Button->OnClicked.RemoveDynamic(this, &UDBAMobaUserWidgetBase::HandleAnyButtonClicked);
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
