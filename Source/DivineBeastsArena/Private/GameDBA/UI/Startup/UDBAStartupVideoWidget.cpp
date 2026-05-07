// Copyright FreeboozStudio. All Rights Reserved.

#include "GameDBA/UI/Startup/UDBAStartupVideoWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "RenderingThread.h"
#include "GameDBA/Core/DBALogChannels.h"

// 浠呭湪闈炴湇鍔″櫒鏋勫缓鏃跺寘鍚?MediaAssets
#if !UE_SERVER
#include "MediaPlayer.h"
#include "MediaTexture.h"
#endif

void UDBAStartupVideoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 璁剧疆璺宠繃鎻愮ず鏂囨湰
	if (SkipHintText)
	{
		SkipHintText->SetText(SkipHint);
	}

	// 缁戝畾璺宠繃鎸夐挳鐐瑰嚮浜嬩欢
	if (SkipButton)
	{
		SkipButton->OnClicked.AddDynamic(this, &UDBAStartupVideoWidget::OnSkipClicked);
	}
}

void UDBAStartupVideoWidget::NativeDestruct()
{
	// 娓呯悊濯掍綋璧勬簮
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
		UE_LOG(LogDBAUI, Log, TEXT("[DBAStartupVideoWidget] 濯掍綋鎾斁鍣ㄥ凡璁剧疆"));
	}
}

void UDBAStartupVideoWidget::OnVideoFinished()
{
	bIsPlaying = false;
	UE_LOG(LogDBAUI, Log, TEXT("[DBAStartupVideoWidget] 瑙嗛鎾斁瀹屾垚"));
}

void UDBAStartupVideoWidget::OnSkipClicked()
{
	UE_LOG(LogDBAUI, Log, TEXT("[DBAStartupVideoWidget] 鐢ㄦ埛鐐瑰嚮璺宠繃"));
}
