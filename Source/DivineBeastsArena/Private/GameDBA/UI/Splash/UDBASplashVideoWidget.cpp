// Copyright FreeboozStudio. All Rights Reserved.

#include "GameDBA/UI/Splash/UDBASplashVideoWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "RenderingThread.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GameInstance/DBAGameInstance.h"

UDBASplashVideoWidget::UDBASplashVideoWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UDBASplashVideoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 设置跳过提示文本
	if (SkipHintText)
	{
		SkipHintText->SetText(FText::FromString(TEXT("按 ESC 跳过")));
	}

	// 自动开始播放视频
	PlayVideo();
}

void UDBASplashVideoWidget::NativeDestruct()
{
	StopVideo();
	Super::NativeDestruct();
}

void UDBASplashVideoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 检查视频是否播放结束
	if (bIsPlaying && MediaPlayer && !MediaPlayer->IsPlaying())
	{
		// 视频已停止且不在播放（可能已结束）
		if (!MediaPlayer->IsLooping())
		{
			OnVideoFinished();
		}
	}
}

FReply UDBASplashVideoWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// ESC 键跳过视频
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		SkipVideo();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UDBASplashVideoWidget::PlayVideo()
{
	if (bIsPlaying || bCompleted)
	{
		return;
	}

	// 创建媒体播放器
	if (!MediaPlayer)
	{
		MediaPlayer = NewObject<UMediaPlayer>(this);
		MediaPlayer->SetLooping(false);
	}

	// 创建媒体纹理
	if (!MediaTexture)
	{
		MediaTexture = NewObject<UMediaTexture>(this);
	}

	// 设置媒体播放器到纹理
	if (MediaTexture && MediaPlayer)
	{
		MediaTexture->SetMediaPlayer(MediaPlayer);

		// 使用文件 URL 打开视频
		FString VideoPath = FPaths::ProjectContentDir() / TEXT("Movies/Startup.mp4");
		FString FileUrl = FString(TEXT("file://")) + VideoPath;

		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 打开视频: %s"), *FileUrl);

		if (MediaPlayer->OpenUrl(FileUrl))
		{
			MediaPlayer->Play();
			bIsPlaying = true;
			UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 开始播放启动视频"));
		}
		else
		{
			UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 无法打开视频文件: %s"), *FileUrl);
		}
	}
}

void UDBASplashVideoWidget::StopVideo()
{
	if (MediaPlayer)
	{
		MediaPlayer->Pause();
		MediaPlayer->Close();
	}
	bIsPlaying = false;
}

void UDBASplashVideoWidget::SkipVideo()
{
	if (bCompleted)
	{
		return;
	}

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 用户跳过视频"));
	StopVideo();
	bCompleted = true;
	TransitionToLogin();
}

void UDBASplashVideoWidget::OnVideoFinished()
{
	bIsPlaying = false;
	bCompleted = true;
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 视频播放完成"));
	TransitionToLogin();
}

void UDBASplashVideoWidget::TransitionToLogin()
{
	// 通知 GameInstance 开始登录流程
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
}