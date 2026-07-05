// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Splash/UDBASplashVideoWidget.h"

#include "Components/Button.h"
#include "Components/AudioComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GameInstance/DBAGameInstance.h"
#include "GameDBA/UI/DBAGameUIManager.h"
#include "Kismet/GameplayStatics.h"
#if !UE_SERVER
#include "FileMediaSource.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaTexture.h"
#endif
#include "Sound/SoundBase.h"
#include "Sound/SoundWaveProcedural.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include <mmsystem.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

namespace
{
	void PlayUISkipButtonClickSound(const UObject* WorldContextObject)
	{
		USoundBase* ClickSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick.SFX_UI_ButtonClick"));
		if (ClickSound)
		{
			UGameplayStatics::PlaySound2D(WorldContextObject, ClickSound, 0.85f, 1.0f, 0.0f, nullptr, nullptr, true);
		}
	}
}

UDBASplashVideoWidget::UDBASplashVideoWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UDBASplashVideoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 原生构造完成"));
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 控件绑定状态：VideoImage=%s，SkipHintText=%s，SkipButton=%s"),
		VideoImage ? *VideoImage->GetName() : TEXT("NULL"),
		SkipHintText ? *SkipHintText->GetName() : TEXT("NULL"),
		SkipButton ? *SkipButton->GetName() : TEXT("NULL"));

	if (SkipHintText)
	{
		SkipHintText->SetText(FText::FromString(TEXT("按 ESC 跳过")));
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] SkipHintText 为空，蓝图绑定可能失败"));
	}

	if (SkipButton)
	{
		SkipButton->OnClicked.AddDynamic(this, &UDBASplashVideoWidget::OnSkipButtonClicked);
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] SkipButton 为空，蓝图绑定可能失败"));
	}

	if (UWorld* World = GetWorld())
	{
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, this, &UDBASplashVideoWidget::PlayVideo, 0.5f, false);
	}

	SetFocus();
}

void UDBASplashVideoWidget::NativeDestruct()
{
	StopVideo();
	Super::NativeDestruct();
}

void UDBASplashVideoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bCompleted)
	{
		ElapsedTime += InDeltaTime;
		if (ElapsedTime > 30.0f)
		{
			UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 启动视频播放超时，继续进入登录流程"));
			OnVideoFinished();
		}
	}
}

FReply UDBASplashVideoWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		SkipVideo();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
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

FReply UDBASplashVideoWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		SkipVideo();
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UDBASplashVideoWidget::OnSkipButtonClicked()
{
	PlayUISkipButtonClickSound(this);
	SkipVideo();
}

void UDBASplashVideoWidget::PlayVideo()
{
	if (bCompleted)
	{
		return;
	}

#if UE_SERVER
	UE_LOG(LogDBAUI, Verbose, TEXT("[UDBASplashVideoWidget] 专用服务器跳过启动视频播放"));
	OnVideoFinished();
	return;
#else
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 开始播放启动视频"));

	if (!MediaPlayer)
	{
		MediaPlayer = TStrongObjectPtr<UMediaPlayer>(NewObject<UMediaPlayer>(this));
	}
	if (!MediaPlayer)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 创建 MediaPlayer 失败"));
		OnVideoFinished();
		return;
	}

	MediaPlayer->SetLooping(false);
	MediaPlayer->OnMediaOpened.RemoveAll(this);
	MediaPlayer->OnMediaOpenFailed.RemoveAll(this);
	MediaPlayer->OnEndReached.RemoveAll(this);
	MediaPlayer->OnMediaOpened.AddDynamic(this, &UDBASplashVideoWidget::HandleMediaOpened);
	MediaPlayer->OnMediaOpenFailed.AddDynamic(this, &UDBASplashVideoWidget::HandleMediaOpenFailed);
	MediaPlayer->OnEndReached.AddDynamic(this, &UDBASplashVideoWidget::HandleMediaEndReached);

	if (!FileMediaSource)
	{
		FileMediaSource = TStrongObjectPtr<UFileMediaSource>(NewObject<UFileMediaSource>(this));
	}
	if (!FileMediaSource)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 创建 FileMediaSource 失败"));
		OnVideoFinished();
		return;
	}

	FString VideoPath = FPaths::ProjectContentDir() / TEXT("Movies/Startup.mp4");
	FPaths::NormalizeFilename(VideoPath);
	if (!FPaths::FileExists(VideoPath))
	{
		FString SourceTreePath = FPaths::ProjectDir() / TEXT("Content/Movies/Startup.mp4");
		FPaths::NormalizeFilename(SourceTreePath);
		if (FPaths::FileExists(SourceTreePath))
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] 暂存启动视频缺失，改用源码目录视频：%s"), *SourceTreePath);
			VideoPath = SourceTreePath;
		}
	}

	const int64 VideoFileSize = IFileManager::Get().FileSize(*VideoPath);
	if (VideoFileSize <= 0)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 启动视频文件缺失或为空：%s"), *VideoPath);
		OnVideoFinished();
		return;
	}

	FileMediaSource->SetFilePath(VideoPath);
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 启动视频路径：%s，大小：%lld 字节"), *VideoPath, VideoFileSize);

	if (!MediaTexture)
	{
		MediaTexture = TStrongObjectPtr<UMediaTexture>(NewObject<UMediaTexture>(this));
	}
	if (!MediaTexture)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 创建 MediaTexture 失败"));
		OnVideoFinished();
		return;
	}

	MediaTexture->SetMediaPlayer(MediaPlayer.Get());
	MediaTexture->UpdateResource();

	if (VideoImage)
	{
		int32 TextureW = MediaTexture->GetWidth();
		int32 TextureH = MediaTexture->GetHeight();
		if (TextureW == 0 || TextureH == 0)
		{
			TextureW = 1920;
			TextureH = 1080;
		}

		FSlateBrush Brush;
		Brush.SetResourceObject(MediaTexture.Get());
		Brush.ImageSize = FVector2D(TextureW, TextureH);
		VideoImage->SetBrush(Brush);
		VideoImage->SetColorAndOpacity(FLinearColor::White);

		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 视频画刷已设置，尺寸：%dx%d"), TextureW, TextureH);
	}
	else
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] VideoImage 为空，蓝图绑定可能失败"));
	}

	if (!MediaSoundComponent)
	{
		MediaSoundComponent = TStrongObjectPtr<UMediaSoundComponent>(NewObject<UMediaSoundComponent>(this));
	}
	if (MediaSoundComponent)
	{
		MediaSoundComponent->bIsUISound = true;
		MediaSoundComponent->bAllowSpatialization = false;
		MediaSoundComponent->SetEnableEnvelopeFollowing(true);
		MediaSoundComponent->SetEnvelopeFollowingsettings(10, 100);
		MediaSoundComponent->SetMediaPlayer(MediaPlayer.Get());
		MediaSoundComponent->AddClockSink();
		if (UWorld* World = GetWorld())
		{
			if (!MediaSoundComponent->IsRegistered())
			{
				MediaSoundComponent->RegisterComponentWithWorld(World);
			}
		}
		MediaSoundComponent->Start();
		MediaSoundComponent->SetVolumeMultiplier(0.0f);

		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 媒体音频组件状态：已注册=%s，播放中=%s"),
			MediaSoundComponent->IsRegistered() ? TEXT("true") : TEXT("false"),
			MediaSoundComponent->IsPlaying() ? TEXT("true") : TEXT("false"));
	}
	else
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 创建 MediaSoundComponent 失败"));
	}

	if (!MediaPlayer->OpenSource(FileMediaSource.Get()))
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 请求打开媒体失败"));
		OnVideoFinished();
		return;
	}

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 已请求打开媒体"));
#endif
}

void UDBASplashVideoWidget::HandleMediaOpened(FString OpenedUrl)
{
#if UE_SERVER
	UE_LOG(LogDBAUI, Verbose, TEXT("[UDBASplashVideoWidget] 专用服务器忽略媒体打开事件：%s"), *OpenedUrl);
	OnVideoFinished();
#else
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 媒体已打开：%s，时长：%.2f 秒"), *OpenedUrl, MediaPlayer ? MediaPlayer->GetDuration().GetTotalSeconds() : 0.0);
	if (MediaSoundComponent)
	{
		MediaSoundComponent->SetMediaPlayer(MediaPlayer.Get());
		if (!MediaSoundComponent->IsPlaying())
		{
			MediaSoundComponent->Start();
		}
		MediaSoundComponent->SetVolumeMultiplier(0.0f);
		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 媒体打开后的音频状态：已注册=%s，播放中=%s"),
			MediaSoundComponent->IsRegistered() ? TEXT("true") : TEXT("false"),
			MediaSoundComponent->IsPlaying() ? TEXT("true") : TEXT("false"));
	}
	PlayFallbackAudio();
	StartPlayback();
#endif
}

void UDBASplashVideoWidget::HandleMediaOpenFailed(FString FailedUrl)
{
#if UE_SERVER
	UE_LOG(LogDBAUI, Verbose, TEXT("[UDBASplashVideoWidget] 专用服务器忽略媒体打开失败事件：%s"), *FailedUrl);
#else
	UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 媒体打开失败：%s"), *FailedUrl);
#endif
	OnVideoFinished();
}

void UDBASplashVideoWidget::HandleMediaEndReached()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 媒体播放已结束"));
	OnVideoFinished();
}

void UDBASplashVideoWidget::StartPlayback()
{
#if UE_SERVER
	return;
#else
	if (!MediaPlayer)
	{
		return;
	}

	const bool bPlayStarted = MediaPlayer->Play();
	bIsPlaying = bPlayStarted || MediaPlayer->IsPlaying();
	ElapsedTime = 0.0f;

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 启动播放请求结果：已开始=%s，播放中=%s，存在视频控件=%s"),
		bPlayStarted ? TEXT("true") : TEXT("false"),
		MediaPlayer->IsPlaying() ? TEXT("true") : TEXT("false"),
		VideoImage ? TEXT("true") : TEXT("false"));

	if (UWorld* World = GetWorld())
	{
		FTimerHandle VerifyTimerHandle;
		World->GetTimerManager().SetTimer(VerifyTimerHandle, this, &UDBASplashVideoWidget::VerifyPlaybackState, 1.0f, false);
	}
#endif
}

void UDBASplashVideoWidget::VerifyPlaybackState()
{
#if UE_SERVER
	return;
#else
	if (!MediaPlayer || bCompleted)
	{
		return;
	}

	if (MediaPlayer->IsReady() && !MediaPlayer->IsPlaying())
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] 媒体已就绪但未播放，正在重试播放"));
		MediaPlayer->Play();
	}

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 延迟检查播放状态：已就绪=%s，播放中=%s，时间=%.2f 秒"),
		MediaPlayer->IsReady() ? TEXT("true") : TEXT("false"),
		MediaPlayer->IsPlaying() ? TEXT("true") : TEXT("false"),
		MediaPlayer->GetTime().GetTotalSeconds());

	if (MediaSoundComponent)
	{
		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 延迟检查音频状态：已注册=%s，播放中=%s，包络=%.4f"),
			MediaSoundComponent->IsRegistered() ? TEXT("true") : TEXT("false"),
			MediaSoundComponent->IsPlaying() ? TEXT("true") : TEXT("false"),
			MediaSoundComponent->GetEnvelopeValue());
	}
#endif
}

void UDBASplashVideoWidget::StopVideo()
{
	if (FallbackAudioComponent)
	{
		FallbackAudioComponent->Stop();
		if (FallbackAudioComponent->IsRegistered())
		{
			FallbackAudioComponent->UnregisterComponent();
		}
		FallbackAudioComponent->DestroyComponent();
		FallbackAudioComponent = nullptr;
	}

#if PLATFORM_WINDOWS
	::PlaySoundW(nullptr, nullptr, 0);
#endif

#if !UE_SERVER
	if (VideoImage)
	{
		VideoImage->SetBrush(FSlateBrush());
	}

	if (MediaSoundComponent)
	{
		MediaSoundComponent->RemoveClockSink();
		MediaSoundComponent->Stop();
		MediaSoundComponent->SetMediaPlayer(nullptr);
		if (MediaSoundComponent->IsRegistered())
		{
			MediaSoundComponent->UnregisterComponent();
		}
		MediaSoundComponent->DestroyComponent();
		MediaSoundComponent.Reset();
	}

	if (MediaPlayer)
	{
		MediaPlayer->OnMediaOpened.RemoveAll(this);
		MediaPlayer->OnMediaOpenFailed.RemoveAll(this);
		MediaPlayer->OnEndReached.RemoveAll(this);
		MediaPlayer->Close();
		MediaPlayer.Reset();
	}

	if (MediaTexture)
	{
		MediaTexture->SetMediaPlayer(nullptr);
		MediaTexture->UpdateResource();
		MediaTexture.Reset();
	}

	FileMediaSource.Reset();
#endif

	FallbackAudioWave = nullptr;
	bIsPlaying = false;
}

void UDBASplashVideoWidget::PlayFallbackAudio()
{
#if UE_SERVER
	return;
#else
	if (FallbackAudioComponent)
	{
		return;
	}

	FString WavPath = FPaths::ProjectContentDir() / TEXT("Movies/Startup.wav");
	FPaths::NormalizeFilename(WavPath);
	if (!FPaths::FileExists(WavPath))
	{
		FString SourceTreePath = FPaths::ProjectDir() / TEXT("Content/Movies/Startup.wav");
		FPaths::NormalizeFilename(SourceTreePath);
		if (FPaths::FileExists(SourceTreePath))
		{
			WavPath = SourceTreePath;
		}
	}

	FallbackAudioWave = CreateSoundWaveFromPcmWav(WavPath);
	if (!FallbackAudioWave)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 创建备用音频波形失败：%s"), *WavPath);
		return;
	}

	FallbackAudioComponent = UGameplayStatics::SpawnSound2D(this, FallbackAudioWave, 1.0f, 1.0f, 0.0f, nullptr, true, false);
	if (FallbackAudioComponent)
	{
		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 备用 Startup.wav 音频播放中：%s"), FallbackAudioComponent->IsPlaying() ? TEXT("true") : TEXT("false"));
	}
	else
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 创建备用 Startup.wav 音频组件失败"));
	}

	PlayNativeFallbackAudio(WavPath);
#endif
}

void UDBASplashVideoWidget::PlayNativeFallbackAudio(const FString& WavPath)
{
#if PLATFORM_WINDOWS
	const FString FullWavPath = FPaths::ConvertRelativePathToFull(WavPath);
	const bool bPlayed = ::PlaySoundW(*FullWavPath, nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Windows 原生 Startup.wav 音频播放中：%s，路径：%s"),
		bPlayed ? TEXT("true") : TEXT("false"),
		*FullWavPath);
#else
	UE_LOG(LogDBAUI, Verbose, TEXT("[UDBASplashVideoWidget] 原生音频备用播放仅在 Windows 平台可用"));
#endif
}

USoundWaveProcedural* UDBASplashVideoWidget::CreateSoundWaveFromPcmWav(const FString& WavPath)
{
	TArray<uint8> WavBytes;
	if (!FFileHelper::LoadFileToArray(WavBytes, *WavPath) || WavBytes.Num() < 44)
	{
		return nullptr;
	}

	auto ReadU16 = [&WavBytes](int32 Offset) -> uint16
	{
		return static_cast<uint16>(WavBytes[Offset] | (WavBytes[Offset + 1] << 8));
	};
	auto ReadU32 = [&WavBytes](int32 Offset) -> uint32
	{
		return static_cast<uint32>(WavBytes[Offset] | (WavBytes[Offset + 1] << 8) | (WavBytes[Offset + 2] << 16) | (WavBytes[Offset + 3] << 24));
	};

	if (FMemory::Memcmp(WavBytes.GetData(), "RIFF", 4) != 0 || FMemory::Memcmp(WavBytes.GetData() + 8, "WAVE", 4) != 0)
	{
		return nullptr;
	}

	int32 ChunkCursor = 12;
	uint16 FormatTag = 0;
	uint16 NumChannels = 0;
	uint32 SampleRate = 0;
	uint16 BitsPerSample = 0;
	int32 DataOffset = INDEX_NONE;
	uint32 DataSize = 0;

	while (ChunkCursor + 8 <= WavBytes.Num())
	{
		const uint32 ChunkSize = ReadU32(ChunkCursor + 4);
		const int32 ChunkDataOffset = ChunkCursor + 8;
		if (ChunkDataOffset + static_cast<int32>(ChunkSize) > WavBytes.Num())
		{
			break;
		}

		if (FMemory::Memcmp(WavBytes.GetData() + ChunkCursor, "fmt ", 4) == 0 && ChunkSize >= 16)
		{
			FormatTag = ReadU16(ChunkDataOffset);
			NumChannels = ReadU16(ChunkDataOffset + 2);
			SampleRate = ReadU32(ChunkDataOffset + 4);
			BitsPerSample = ReadU16(ChunkDataOffset + 14);
		}
		else if (FMemory::Memcmp(WavBytes.GetData() + ChunkCursor, "data", 4) == 0)
		{
			DataOffset = ChunkDataOffset;
			DataSize = ChunkSize;
			break;
		}

		ChunkCursor = ChunkDataOffset + static_cast<int32>(ChunkSize) + (ChunkSize % 2);
	}

	if (FormatTag != 1 || NumChannels == 0 || SampleRate == 0 || BitsPerSample != 16 || DataOffset == INDEX_NONE || DataSize == 0)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] 不支持的 WAV 格式：格式=%u，声道=%u，采样率=%u，位深=%u，数据大小=%u"),
			FormatTag, NumChannels, SampleRate, BitsPerSample, DataSize);
		return nullptr;
	}

	USoundWaveProcedural* SoundWave = NewObject<USoundWaveProcedural>(this);
	if (!SoundWave)
	{
		return nullptr;
	}

	SoundWave->SetSampleRate(static_cast<int32>(SampleRate));
	SoundWave->NumChannels = NumChannels;
	SoundWave->Duration = static_cast<float>(DataSize) / static_cast<float>(SampleRate * NumChannels * sizeof(int16));
	SoundWave->SoundGroup = SOUNDGROUP_UI;
	SoundWave->bLooping = false;
	SoundWave->QueueAudio(WavBytes.GetData() + DataOffset, DataSize);

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 已加载备用 WAV：%s，%.2f 秒，%u Hz，%u 声道，%u 字节"),
		*WavPath, SoundWave->Duration, SampleRate, NumChannels, DataSize);

	return SoundWave;
}

void UDBASplashVideoWidget::SkipVideo()
{
	if (bCompleted)
	{
		return;
	}

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 跳过启动视频"));
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

	StopVideo();
	bCompleted = true;
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 启动视频流程已完成"));
	TransitionToLogin();
}

void UDBASplashVideoWidget::TransitionToLogin()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] 切换到登录流程"));
	UWorld* ResolvedWorld = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		ResolvedWorld = PC->GetWorld();
	}
	if (!ResolvedWorld)
	{
		ResolvedWorld = GetWorld();
	}

	UDBAGameInstance* GI = ResolvedWorld ? Cast<UDBAGameInstance>(ResolvedWorld->GetGameInstance()) : nullptr;

	RemoveSelf();

	if (GI)
	{
		GI->StartLoginFlow();
		if (UDBAGameUIManager* UIManager = GI->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->RequestShowLoginFlowWidget();
		}
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] 切换到登录流程时无法解析 DBA GameInstance"));
	}
}

void UDBASplashVideoWidget::RemoveSelf()
{
	if (IsInViewport())
	{
		RemoveFromParent();
	}
}
