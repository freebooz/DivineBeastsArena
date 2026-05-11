// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "UDBASplashVideoWidget.generated.h"

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

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION(BlueprintCallable, Category = "SplashVideo")
	void PlayVideo();

	UFUNCTION(BlueprintCallable, Category = "SplashVideo")
	void StopVideo();

	UFUNCTION(BlueprintCallable, Category = "SplashVideo")
	void SkipVideo();

private:
	void OnVideoFinished();
	void TransitionToLogin();

private:
	/** 媒体播放器 */
	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> MediaPlayer;

	/** 媒体纹理 */
	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> MediaTexture;

	/** 视频显示图像 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> VideoImage;

	/** 跳过提示文本 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> SkipHintText;

	/** 是否正在播放 */
	UPROPERTY(Transient)
	bool bIsPlaying = false;

	/** 是否已处理完成 */
	UPROPERTY(Transient)
	bool bCompleted = false;
};