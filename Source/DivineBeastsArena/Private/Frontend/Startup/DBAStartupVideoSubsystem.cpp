// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/Startup/DBAStartupVideoSubsystem.h"
#include "Common/Subsystems/DBASubsystemImpl.h"
#include "Common/DBALogChannels.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

// 媒体资源仅在非专用服务器构建时可用
#if !UE_SERVER
#include "MediaPlayer.h"
#include "MediaTexture.h"
#endif

UDBAStartupVideoSubsystem::UDBAStartupVideoSubsystem()
	: CurrentState(EDBAStartupVideoState::NotStarted)
{
}

void UDBAStartupVideoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LogSubsystemInfo(TEXT("初始化完成"));
}

void UDBAStartupVideoSubsystem::Deinitialize()
{
	// 清理定时器
	if (VideoFinishedTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(VideoFinishedTimerHandle);
		}
	}

	// 移除 ESC 绑定
	if (bESCBound)
	{
		HandleESCInput();
	}

	Super::Deinitialize();
}

bool UDBAStartupVideoSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// 仅在客户端创建
	if (const UGameInstance* GameInstance = Cast<UGameInstance>(Outer))
	{
		return GameInstance->GetWorld() && !GameInstance->GetWorld()->IsNetMode(NM_DedicatedServer);
	}
	return false;
}

void UDBAStartupVideoSubsystem::PlayStartupVideo()
{
	if (CurrentState != EDBAStartupVideoState::NotStarted)
	{
		LogSubsystemWarning(TEXT("视频已在播放或已完成"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		LogSubsystemError(TEXT("无法获取 World"));
		return;
	}

	// 仅客户端播放
	if (World->IsNetMode(NM_DedicatedServer))
	{
		LogSubsystemInfo(TEXT("专用服务器跳过启动视频"));
		CurrentState = EDBAStartupVideoState::Finished;
		return;
	}

	CurrentState = EDBAStartupVideoState::Playing;
	LogSubsystemInfo(TEXT("开始播放启动视频"));

#if !UE_SERVER
	// 创建视频播放器
	MediaPlayer = NewObject<UMediaPlayer>(this);
	if (MediaPlayer)
	{
		MediaPlayer->PlayOnOpen = true;
		MediaPlayer->SetLooping(false);

		// 打开视频
		FSoftObjectPath VideoPath(StartupVideoPath);
		MediaPlayer->OpenUrl(VideoPath.ToString());

		LogSubsystemInfo(FString::Printf(TEXT("视频路径: %s"), *StartupVideoPath));
	}

	// 创建视频播放控件
	CreateVideoWidget();
#endif

	// 设置超时定时器
	World->GetTimerManager().SetTimer(VideoFinishedTimerHandle, this, &UDBAStartupVideoSubsystem::OnVideoFinished, VideoTimeout, false);

	// 绑定 ESC 输入
	HandleESCInput();
}

void UDBAStartupVideoSubsystem::CreateVideoWidget()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	// 获取 PlayerController
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		LogSubsystemError(TEXT("无法获取 PlayerController"));
		return;
	}

	// 添加到视口
	if (VideoWidget)
	{
		VideoWidget->AddToViewport();
		LogSubsystemInfo(TEXT("视频控件已添加到视口"));
	}
}

void UDBAStartupVideoSubsystem::OnVideoFinished()
{
	if (CurrentState == EDBAStartupVideoState::Playing)
	{
		LogSubsystemInfo(TEXT("视频播放完成"));
		CurrentState = EDBAStartupVideoState::Finished;
	}

	NavigateToLoginScreen();
}

void UDBAStartupVideoSubsystem::SkipVideo()
{
	if (CurrentState != EDBAStartupVideoState::Playing)
	{
		return;
	}

	LogSubsystemInfo(TEXT("用户跳过视频"));

#if !UE_SERVER
	// 停止播放
	if (MediaPlayer)
	{
		MediaPlayer->Pause();
	}
#endif

	// 清理定时器
	if (VideoFinishedTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(VideoFinishedTimerHandle);
		}
	}

	CurrentState = EDBAStartupVideoState::Skipped;
	NavigateToLoginScreen();
}

void UDBAStartupVideoSubsystem::HandleESCInput()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
		return;

	if (bESCBound)
	{
		bESCBound = false;
		LogSubsystemInfo(TEXT("已解除 ESC 绑定"));
	}
	else
	{
		bESCBound = true;
		LogSubsystemInfo(TEXT("已绑定 ESC 到 SkipVideo"));
	}
}

void UDBAStartupVideoSubsystem::NavigateToLoginScreen()
{
	LogSubsystemInfo(TEXT("跳转到登录界面"));

	// 移除视频控件
	if (VideoWidget)
	{
		VideoWidget->RemoveFromParent();
		VideoWidget = nullptr;
	}

	// 解除 ESC 绑定
	if (bESCBound)
	{
		HandleESCInput();
	}

	// 广播视频完成事件，让前端系统加载登录界面
	StartupVideoFinishedEvent.Broadcast(CurrentState);
	LogSubsystemInfo(TEXT("已广播 StartupVideoFinished 事件"));
}