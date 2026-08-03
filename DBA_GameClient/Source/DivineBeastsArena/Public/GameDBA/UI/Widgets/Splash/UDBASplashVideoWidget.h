// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UObject/StrongObjectPtr.h"
#if !UE_SERVER
#include "FileMediaSource.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaTexture.h"
#endif
#include "UDBASplashVideoWidget.generated.h"

class UAudioComponent;
class USoundWaveProcedural;

/**
 * UDBASplashVideoWidget
 *
 * 启动视频播放控件 (类似魔兽世界开场视频)
 * - 自动播放 Startup.mp4
 * - 按 ESC 可跳过
 * - 视频结束或跳过后显示登录界面
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBASplashVideoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDBASplashVideoWidget(const FObjectInitializer& ObjectInitializer);

	/** 将启动视频控件与视频画面铺满游戏视口（不使用 RenderScale） */
	UFUNCTION(BlueprintCallable, Category = "SplashVideo")
	void ApplySplashFullscreenPresentation();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION(BlueprintCallable, Category = "SplashVideo")
	void PlayVideo();

	UFUNCTION(BlueprintCallable, Category = "SplashVideo")
	void StopVideo();

	UFUNCTION(BlueprintCallable, Category = "SplashVideo")
	void SkipVideo();

private:
	UFUNCTION()
	void OnSkipButtonClicked();
	UFUNCTION()
	void HandleMediaOpened(FString OpenedUrl);
	UFUNCTION()
	void HandleMediaOpenFailed(FString FailedUrl);
	UFUNCTION()
	void HandleMediaEndReached();
	UFUNCTION()
	void VerifyPlaybackState();

	void OnVideoFinished();
	void TransitionToLogin();
	void RemoveSelf();
	void StartPlayback();
	void PlayFallbackAudio();
	void PlayNativeFallbackAudio(const FString& WavPath);
	USoundWaveProcedural* CreateSoundWaveFromPcmWav(const FString& WavPath);

private:
#if !UE_SERVER
	/** 媒体播放器 */
	TStrongObjectPtr<UMediaPlayer> MediaPlayer;

	/** 媒体纹理 */
	TStrongObjectPtr<UMediaTexture> MediaTexture;

	TStrongObjectPtr<UFileMediaSource> FileMediaSource;

	TStrongObjectPtr<UMediaSoundComponent> MediaSoundComponent;
#endif

	UPROPERTY(Transient)
	TObjectPtr<USoundWaveProcedural> FallbackAudioWave;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> FallbackAudioComponent;

	/** 视频显示图像 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> VideoImage;

	/** 跳过提示文本 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SkipHintText;

	/** 跳过按钮 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SkipButton;

	/** 是否正在播放 */
	UPROPERTY(Transient)
	bool bIsPlaying = false;

	/** 是否已处理完成 */
	UPROPERTY(Transient)
	bool bCompleted = false;

	/** 已过时间 */
	UPROPERTY(Transient)
	float ElapsedTime = 0.0f;

	/** 是否已完成一次延迟全屏校正（避免蓝图布局覆盖锚点） */
	UPROPERTY(Transient)
	bool bAppliedDelayedFullscreenLayout = false;
};
