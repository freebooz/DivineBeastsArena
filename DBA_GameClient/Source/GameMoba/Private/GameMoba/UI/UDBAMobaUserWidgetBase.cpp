// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "GameMoba/UI/DBAMobaUIDeveloperSettings.h"
#include "GameCore/Core/DBALogChannels.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "UObject/SoftObjectPath.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

UDBAMobaUserWidgetBase::UDBAMobaUserWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 从 DeveloperSettings 读取默认软引用，避免构造函数硬编码资源路径。
	if (const UDBAMobaUIDeveloperSettings* Settings = GetDefault<UDBAMobaUIDeveloperSettings>())
	{
		DefaultClickSound = Settings->DefaultClickSound;
		DefaultBackgroundTexture = Settings->DefaultBackgroundTexture;
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBAMobaUserWidgetBase] 无法读取 DBA Moba UI 配置，默认点击音效与背景纹理软引用为空。"));
	}
}

void UDBAMobaUserWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (bAutoInjectBackground)
	{
		ApplyDefaultBackgroundTexture();
	}

	// 异步预加载默认点击音效，避免按钮点击时同步加载阻塞 GameThread。
	PreloadDefaultClickSound();
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

	// 取消未完成的异步加载句柄，避免回调悬挂。
	if (ClickSoundStreamableHandle.IsValid())
	{
		ClickSoundStreamableHandle->CancelHandle();
		ClickSoundStreamableHandle.Reset();
	}

	Super::NativeDestruct();
}

void UDBAMobaUserWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UDBAMobaUserWidgetBase::HandleAnyButtonClicked()
{
	USoundBase* ClickSound = CachedClickSound.Get();
	if (!ClickSound)
	{
		// 音效尚未异步加载完成，输出一次中文警告日志，避免日志刷屏。
		if (!bHasLoggedClickSoundNotReady)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[UDBAMobaUserWidgetBase] 默认点击音效尚未异步加载完成，本次点击跳过音效播放。软引用路径：%s"),
				DefaultClickSound.IsValid() ? TEXT("已加载但弱指针失效") : *DefaultClickSound.ToSoftObjectPath().ToString());
			bHasLoggedClickSoundNotReady = true;
		}
		return;
	}

	// 重置警告标记，便于下次未就绪时再次提示。
	bHasLoggedClickSoundNotReady = false;
	UGameplayStatics::PlaySound2D(this, ClickSound, 0.85f, 1.0f, 0.0f, nullptr, nullptr, true);
}

void UDBAMobaUserWidgetBase::PreloadDefaultClickSound()
{
	// 若已缓存或软引用无效，直接返回。
	if (CachedClickSound.IsValid())
	{
		return;
	}

	if (!DefaultClickSound.IsValid())
	{
		// 软引用尚未加载，发起异步加载。
		const FSoftObjectPath& SoundPath = DefaultClickSound.ToSoftObjectPath();
		if (!SoundPath.IsValid())
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[UDBAMobaUserWidgetBase] DefaultClickSound 软引用路径无效，无法异步加载。"));
			return;
		}

		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		ClickSoundStreamableHandle = Streamable.RequestAsyncLoad(
			SoundPath,
			FStreamableDelegate::CreateUObject(this, &UDBAMobaUserWidgetBase::HandleDefaultClickSoundLoaded));
	}
	else
	{
		// 软引用已加载，直接缓存。
		HandleDefaultClickSoundLoaded();
	}
}

void UDBAMobaUserWidgetBase::HandleDefaultClickSoundLoaded()
{
	ClickSoundStreamableHandle.Reset();

	USoundBase* LoadedSound = DefaultClickSound.Get();
	if (!LoadedSound)
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBAMobaUserWidgetBase] 默认点击音效异步加载完成但解析为空，请检查软引用路径：%s"),
			*DefaultClickSound.ToSoftObjectPath().ToString());
		return;
	}

	CachedClickSound = LoadedSound;
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAMobaUserWidgetBase] 默认点击音效异步加载完成并已缓存：%s"),
		*LoadedSound->GetName());
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
