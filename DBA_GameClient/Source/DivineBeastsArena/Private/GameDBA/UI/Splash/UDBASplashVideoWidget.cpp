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
#include "FileMediaSource.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GameInstance/DBAGameInstance.h"
#include "GameDBA/UI/DBAGameUIManager.h"
#include "Kismet/GameplayStatics.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaTexture.h"
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

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] NativeConstruct"));
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] VideoImage: %s, SkipHintText: %s, SkipButton: %s"),
		VideoImage ? *VideoImage->GetName() : TEXT("NULL"),
		SkipHintText ? *SkipHintText->GetName() : TEXT("NULL"),
		SkipButton ? *SkipButton->GetName() : TEXT("NULL"));

	if (SkipHintText)
	{
		SkipHintText->SetText(FText::FromString(TEXT("Press ESC to skip")));
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] SkipHintText is NULL - Blueprint binding may have failed"));
	}

	if (SkipButton)
	{
		SkipButton->OnClicked.AddDynamic(this, &UDBASplashVideoWidget::OnSkipButtonClicked);
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] SkipButton is NULL - Blueprint binding may have failed"));
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
			UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Timed out, continuing to login"));
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

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] PlayVideo"));

	if (!MediaPlayer)
	{
		MediaPlayer = NewObject<UMediaPlayer>(this);
	}
	if (!MediaPlayer)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] Failed to create MediaPlayer"));
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
		FileMediaSource = NewObject<UFileMediaSource>(this);
	}
	if (!FileMediaSource)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] Failed to create FileMediaSource"));
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
			UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] Staged movie missing, using source path: %s"), *SourceTreePath);
			VideoPath = SourceTreePath;
		}
	}

	const int64 VideoFileSize = IFileManager::Get().FileSize(*VideoPath);
	if (VideoFileSize <= 0)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] Startup movie file missing or empty: %s"), *VideoPath);
		OnVideoFinished();
		return;
	}

	FileMediaSource->SetFilePath(VideoPath);
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Startup movie path: %s, size: %lld bytes"), *VideoPath, VideoFileSize);

	if (!MediaTexture)
	{
		MediaTexture = NewObject<UMediaTexture>(this);
	}
	if (!MediaTexture)
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] Failed to create MediaTexture"));
		OnVideoFinished();
		return;
	}

	MediaTexture->SetMediaPlayer(MediaPlayer);
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
		Brush.SetResourceObject(MediaTexture);
		Brush.ImageSize = FVector2D(TextureW, TextureH);
		VideoImage->SetBrush(Brush);
		VideoImage->SetColorAndOpacity(FLinearColor::White);

		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Video brush set, size: %dx%d"), TextureW, TextureH);
	}
	else
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] VideoImage is NULL - Blueprint binding may have failed"));
	}

	if (!MediaSoundComponent)
	{
		MediaSoundComponent = NewObject<UMediaSoundComponent>(this);
	}
	if (MediaSoundComponent)
	{
		MediaSoundComponent->bIsUISound = true;
		MediaSoundComponent->bAllowSpatialization = false;
		MediaSoundComponent->SetEnableEnvelopeFollowing(true);
		MediaSoundComponent->SetEnvelopeFollowingsettings(10, 100);
		MediaSoundComponent->SetMediaPlayer(MediaPlayer);
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

		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] MediaSoundComponent registered: %s, playing: %s"),
			MediaSoundComponent->IsRegistered() ? TEXT("true") : TEXT("false"),
			MediaSoundComponent->IsPlaying() ? TEXT("true") : TEXT("false"));
	}
	else
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] Failed to create MediaSoundComponent"));
	}

	if (!MediaPlayer->OpenSource(FileMediaSource))
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] Failed to request media open"));
		OnVideoFinished();
		return;
	}

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Media open requested"));
}

void UDBASplashVideoWidget::HandleMediaOpened(FString OpenedUrl)
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Media opened: %s, duration: %.2fs"), *OpenedUrl, MediaPlayer ? MediaPlayer->GetDuration().GetTotalSeconds() : 0.0);
	if (MediaSoundComponent)
	{
		MediaSoundComponent->SetMediaPlayer(MediaPlayer);
		if (!MediaSoundComponent->IsPlaying())
		{
			MediaSoundComponent->Start();
		}
		MediaSoundComponent->SetVolumeMultiplier(0.0f);
		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Media sound after open: registered=%s, playing=%s"),
			MediaSoundComponent->IsRegistered() ? TEXT("true") : TEXT("false"),
			MediaSoundComponent->IsPlaying() ? TEXT("true") : TEXT("false"));
	}
	PlayFallbackAudio();
	StartPlayback();
}

void UDBASplashVideoWidget::HandleMediaOpenFailed(FString FailedUrl)
{
	UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] Media open failed: %s"), *FailedUrl);
	OnVideoFinished();
}

void UDBASplashVideoWidget::HandleMediaEndReached()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Media end reached"));
	OnVideoFinished();
}

void UDBASplashVideoWidget::StartPlayback()
{
	if (!MediaPlayer)
	{
		return;
	}

	const bool bPlayStarted = MediaPlayer->Play();
	bIsPlaying = bPlayStarted || MediaPlayer->IsPlaying();
	ElapsedTime = 0.0f;

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Playback started: %s, IsPlaying: %s, HasVideoImage: %s"),
		bPlayStarted ? TEXT("true") : TEXT("false"),
		MediaPlayer->IsPlaying() ? TEXT("true") : TEXT("false"),
		VideoImage ? TEXT("true") : TEXT("false"));

	if (UWorld* World = GetWorld())
	{
		FTimerHandle VerifyTimerHandle;
		World->GetTimerManager().SetTimer(VerifyTimerHandle, this, &UDBASplashVideoWidget::VerifyPlaybackState, 1.0f, false);
	}
}

void UDBASplashVideoWidget::VerifyPlaybackState()
{
	if (!MediaPlayer || bCompleted)
	{
		return;
	}

	if (MediaPlayer->IsReady() && !MediaPlayer->IsPlaying())
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] Media was ready but not playing, retrying playback"));
		MediaPlayer->Play();
	}

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Playback state after delay: IsReady=%s, IsPlaying=%s, Time=%.2fs"),
		MediaPlayer->IsReady() ? TEXT("true") : TEXT("false"),
		MediaPlayer->IsPlaying() ? TEXT("true") : TEXT("false"),
		MediaPlayer->GetTime().GetTotalSeconds());

	if (MediaSoundComponent)
	{
		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Sound state after delay: registered=%s, playing=%s, envelope=%.4f"),
			MediaSoundComponent->IsRegistered() ? TEXT("true") : TEXT("false"),
			MediaSoundComponent->IsPlaying() ? TEXT("true") : TEXT("false"),
			MediaSoundComponent->GetEnvelopeValue());
	}
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
		MediaSoundComponent = nullptr;
	}

	if (MediaPlayer)
	{
		MediaPlayer->OnMediaOpened.RemoveAll(this);
		MediaPlayer->OnMediaOpenFailed.RemoveAll(this);
		MediaPlayer->OnEndReached.RemoveAll(this);
		MediaPlayer->Close();
	}

	bIsPlaying = false;
}

void UDBASplashVideoWidget::PlayFallbackAudio()
{
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
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] Failed to create fallback audio wave: %s"), *WavPath);
		return;
	}

	FallbackAudioComponent = UGameplayStatics::SpawnSound2D(this, FallbackAudioWave, 1.0f, 1.0f, 0.0f, nullptr, true, false);
	if (FallbackAudioComponent)
	{
		UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Fallback Startup.wav audio playing: %s"), FallbackAudioComponent->IsPlaying() ? TEXT("true") : TEXT("false"));
	}
	else
	{
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] Failed to spawn fallback Startup.wav audio"));
	}

	PlayNativeFallbackAudio(WavPath);
}

void UDBASplashVideoWidget::PlayNativeFallbackAudio(const FString& WavPath)
{
#if PLATFORM_WINDOWS
	const FString FullWavPath = FPaths::ConvertRelativePathToFull(WavPath);
	const bool bPlayed = ::PlaySoundW(*FullWavPath, nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Native Windows Startup.wav audio playing: %s, path: %s"),
		bPlayed ? TEXT("true") : TEXT("false"),
		*FullWavPath);
#else
	UE_LOG(LogDBAUI, Verbose, TEXT("[UDBASplashVideoWidget] Native audio fallback is only available on Windows"));
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
		UE_LOG(LogDBAUI, Error, TEXT("[UDBASplashVideoWidget] Unsupported WAV format: Format=%u Channels=%u SampleRate=%u Bits=%u DataSize=%u"),
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

	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] Loaded fallback WAV: %s, %.2fs, %u Hz, %u channels, %u bytes"),
		*WavPath, SoundWave->Duration, SampleRate, NumChannels, DataSize);

	return SoundWave;
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

	StopVideo();
	bCompleted = true;
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] OnVideoFinished"));
	TransitionToLogin();
}

void UDBASplashVideoWidget::TransitionToLogin()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBASplashVideoWidget] TransitionToLogin"));
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
		UE_LOG(LogDBAUI, Warning, TEXT("[UDBASplashVideoWidget] Unable to resolve DBA game instance when transitioning to login."));
	}
}

void UDBASplashVideoWidget::RemoveSelf()
{
	if (IsInViewport())
	{
		RemoveFromParent();
	}
}
