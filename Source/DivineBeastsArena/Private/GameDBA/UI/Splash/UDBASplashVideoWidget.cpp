// Copyright FreeboozStudio. All Rights Reserved.

#include "GameDBA/UI/Splash/UDBASplashVideoWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "RenderingThread.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GameInstance/DBAGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "MediaSource.h"
#include "FileMediaSource.h"

UDBASplashVideoWidget::UDBASplashVideoWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UDBASplashVideoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] NativeConstruct"));

	// 检查所有绑定
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] VideoImage: %s, SkipHintText: %s, SkipButton: %s"),
		VideoImage ? *VideoImage->GetName() : TEXT("NULL"),
		SkipHintText ? *SkipHintText->GetName() : TEXT("NULL"),
		SkipButton ? *SkipButton->GetName() : TEXT("NULL"));

	// 设置跳过提示文本
	if (SkipHintText)
	{
		SkipHintText->SetText(FText::FromString(TEXT("按 ESC 跳过")));
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] SkipHintText is NULL - Blueprint binding may have failed"));
	}

	// 绑定跳过按钮
	if (SkipButton)
	{
		SkipButton->OnClicked.AddDynamic(this, &UDBASplashVideoWidget::OnSkipButtonClicked);
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] SkipButton is NULL - Blueprint binding may have failed"));
	}

	// 延迟播放视频
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDBASplashVideoWidget::PlayVideo, 0.5f, false);
}

void UDBASplashVideoWidget::NativeDestruct()
{
	StopVideo();
	Super::NativeDestruct();
}

void UDBASplashVideoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 超时保护
	if (!bCompleted)
	{
		ElapsedTime += InDeltaTime;
		if (ElapsedTime > 30.0f)
		{
			UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 超时自动跳转"));
			OnVideoFinished();
		}
	}
}

FReply UDBASplashVideoWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		SkipVideo();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void UDBASplashVideoWidget::OnSkipButtonClicked()
{
	SkipVideo();
}

void UDBASplashVideoWidget::PlayVideo()
{
	if (bCompleted)
	{
		return;
	}

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] PlayVideo"));

	// 创建媒体播放器
	if (!MediaPlayer)
	{
		MediaPlayer = NewObject<UMediaPlayer>(this);
	}
	if (!MediaPlayer)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 无法创建 MediaPlayer"));
		OnVideoFinished();
		return;
	}

	MediaPlayer->SetLooping(false);

	// 创建文件媒体源
	UFileMediaSource* FileSource = NewObject<UFileMediaSource>(this);
	if (!FileSource)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 无法创建 FileMediaSource"));
		OnVideoFinished();
		return;
	}

	// 设置视频文件路径
	FString VideoPath = FPaths::ProjectContentDir() / TEXT("Movies/Startup.mp4");
	FPaths::NormalizeFilename(VideoPath);
	FileSource->SetFilePath(VideoPath);
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 视频路径: %s"), *VideoPath);

	// 打开媒体源
	if (!MediaPlayer->OpenSource(FileSource))
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 无法打开媒体源"));
		OnVideoFinished();
		return;
	}

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 媒体源打开成功"));

	// 创建媒体纹理
	if (!MediaTexture)
	{
		MediaTexture = NewObject<UMediaTexture>(this);
	}
	if (!MediaTexture)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 无法创建 MediaTexture"));
		OnVideoFinished();
		return;
	}

	MediaTexture->SetMediaPlayer(MediaPlayer);
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] MediaTexture 设置成功, 尺寸: %dx%d"), MediaTexture->GetWidth(), MediaTexture->GetHeight());

	// 开始播放前等待一下让纹理初始化

	// 设置到 Image 控件 - 直接使用 MediaTexture
	if (VideoImage)
	{
		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] VideoImage 有效，准备设置纹理"));

		// 获取视频尺寸
		int32 TextureW = MediaTexture->GetWidth();
		int32 TextureH = MediaTexture->GetHeight();
		if (TextureW == 0 || TextureH == 0)
		{
			TextureW = 1920;
			TextureH = 1080;
		}

		// 直接使用 MediaTexture 作为 Image 的 Brush 资源
		FSlateBrush Brush;
		Brush.SetResourceObject(MediaTexture);
		Brush.ImageSize = FVector2D(TextureW, TextureH);
		VideoImage->SetBrush(Brush);

		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 纹理设置成功, Size: %dx%d"), TextureW, TextureH);
	}
	else
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] VideoImage 为空"));
	}

	// 开始播放前等待一下让纹理初始化
	FTimerHandle PlayTimer;
	GetWorld()->GetTimerManager().SetTimer(PlayTimer, this, &UDBASplashVideoWidget::StartPlayback, 0.3f, false);
}

void UDBASplashVideoWidget::StartPlayback()
{
	if (!MediaPlayer || !VideoImage)
	{
		return;
	}

	MediaPlayer->Play();
	bIsPlaying = true;
	ElapsedTime = 0.0f;
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 开始播放, IsPlaying: %s"), MediaPlayer->IsPlaying() ? TEXT("true") : TEXT("false"));
}

void UDBASplashVideoWidget::StopVideo()
{
	if (MediaPlayer)
	{
		MediaPlayer->Pause();
	}
	bIsPlaying = false;
}

void UDBASplashVideoWidget::SkipVideo()
{
	if (bCompleted)
	{
		return;
	}

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] SkipVideo"));
	StopVideo();
	bCompleted = true;
	TransitionToLogin();
}

void UDBASplashVideoWidget::OnVideoFinished()
{
	if (bCompleted)
	{
		return;
	}

	bIsPlaying = false;
	bCompleted = true;
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] OnVideoFinished"));
	TransitionToLogin();
}

void UDBASplashVideoWidget::TransitionToLogin()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] TransitionToLogin"));

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UWorld* World = PC->GetWorld())
		{
			if (UDBAGameInstance* GI = Cast<UDBAGameInstance>(World->GetGameInstance()))
			{
				GI->StartLoginFlow();
			}
		}
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDBASplashVideoWidget::RemoveSelf, 0.1f, false);
}

void UDBASplashVideoWidget::RemoveSelf()
{
	RemoveFromParent();
}