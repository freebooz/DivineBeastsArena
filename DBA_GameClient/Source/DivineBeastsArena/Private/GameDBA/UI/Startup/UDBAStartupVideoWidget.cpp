// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Startup/UDBAStartupVideoWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "RenderingThread.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// 仅在非服务端构建时包含MediaAssets
#if !UE_SERVER
#include "MediaPlayer.h"
#include "MediaTexture.h"
#endif

namespace
{
	void PlayUIStartupButtonClickSound(const UObject* WorldContextObject)
	{
		USoundBase* ClickSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick.SFX_UI_ButtonClick"));
		if (ClickSound)
		{
			UGameplayStatics::PlaySound2D(WorldContextObject, ClickSound, 0.85f, 1.0f, 0.0f, nullptr, nullptr, true);
		}
	}
}

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
		MediaPlayer->Close();
		MediaPlayer.Reset();
	}
#endif

	bIsPlaying = false;
	Super::NativeDestruct();
}

void UDBAStartupVideoWidget::SetMediaPlayer(UObject* InMediaPlayer)
{
#if UE_SERVER
	UE_LOG(LogDBAUI, Verbose, TEXT("[DBAStartupVideoWidget] 专用服务器忽略媒体播放器设置"));
#else
	MediaPlayer = TStrongObjectPtr<UMediaPlayer>(Cast<UMediaPlayer>(InMediaPlayer));

	if (MediaPlayer)
	{
		bIsPlaying = true;
		UE_LOG(LogDBAUI, Log, TEXT("[DBAStartupVideoWidget] 媒体播放器已设置"));
	}
#endif
}

void UDBAStartupVideoWidget::OnVideoFinished()
{
	bIsPlaying = false;
	UE_LOG(LogDBAUI, Log, TEXT("[DBAStartupVideoWidget] 视频播放完成"));
}

void UDBAStartupVideoWidget::OnSkipClicked()
{
	PlayUIStartupButtonClickSound(this);
	UE_LOG(LogDBAUI, Log, TEXT("[DBAStartupVideoWidget] 用户点击跳过"));
}
