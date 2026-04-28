// Copyright FreeboozStudio. All Rights Reserved.

#include "UI/Startup/DBAStartupVideoWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "RenderingThread.h"
#include "Common/DBALogChannels.h"

// 仅在非服务器构建时包含 MediaAssets
#if !UE_SERVER
#include "MediaPlayer.h"
#include "MediaTexture.h"
#endif

void UDBAStartupVideoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 设置跳过提示文本
	if (SkipHintText)
	{
		SkipHintText->SetText(SkipHint);
	}

	// 绑定跳过按钮点击事件
	if (SkipButton)
	{
		SkipButton->OnClicked.AddDynamic(this, &UDBAStartupVideoWidget::OnSkipClicked);
	}
}

void UDBAStartupVideoWidget::NativeDestruct()
{
	// 清理媒体资源
#if !UE_SERVER
	if (MediaPlayer)
	{
		MediaPlayer->Pause();
	}
#endif

	Super::NativeDestruct();
}

void UDBAStartupVideoWidget::SetMediaPlayer(UMediaPlayer* InMediaPlayer)
{
	MediaPlayer = InMediaPlayer;

	if (MediaPlayer)
	{
		bIsPlaying = true;
		UE_LOG(LogDBAUI, Log, TEXT("[DBAStartupVideoWidget] 媒体播放器已设置"));
	}
}

void UDBAStartupVideoWidget::OnVideoFinished()
{
	bIsPlaying = false;
	UE_LOG(LogDBAUI, Log, TEXT("[DBAStartupVideoWidget] 视频播放完成"));
}

void UDBAStartupVideoWidget::OnSkipClicked()
{
	UE_LOG(LogDBAUI, Log, TEXT("[DBAStartupVideoWidget] 用户点击跳过"));
}