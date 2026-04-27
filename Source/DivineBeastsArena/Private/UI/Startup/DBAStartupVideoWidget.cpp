// Copyright FreeboozStudio. All Rights Reserved.

#include "UI/Startup/DBAStartupVideoWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "RenderingThread.h"
#include "Common/DBALogChannels.h"

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
	if (MediaPlayer)
	{
		MediaPlayer->Pause();
	}

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