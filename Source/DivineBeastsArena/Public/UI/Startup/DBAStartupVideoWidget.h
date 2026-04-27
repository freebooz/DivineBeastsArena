// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "DBAStartupVideoWidget.generated.h"

class UMediaPlayer;
class UMediaTexture;
class UImage;
class UTextBlock;
class UButton;

/**
 * DBAStartupVideoWidget
 *
 * 启动视频播放控件
 * 显示视频内容，并提供跳过提示
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBAStartupVideoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 设置媒体播放器
	 */
	UFUNCTION(BlueprintCallable, Category = "StartupVideo")
	void SetMediaPlayer(UMediaPlayer* InMediaPlayer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * 视频播放完成时调用
	 */
	UFUNCTION(BlueprintCallable, Category = "StartupVideo")
	void OnVideoFinished();

	/**
	 * 跳过的点击事件
	 */
	UFUNCTION(BlueprintCallable, Category = "StartupVideo")
	void OnSkipClicked();

private:
	/** 媒体播放器 */
	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> MediaPlayer;

	/** 跳过提示文本 */
	UPROPERTY( meta = (BindWidget) )
	TObjectPtr<UTextBlock> SkipHintText;

	/** 跳过按钮（透明全屏） */
	UPROPERTY( meta = (BindWidget) )
	TObjectPtr<UButton> SkipButton;

	/** 提示文字 */
	UPROPERTY(EditDefaultsOnly, Category = "StartupVideo")
	FText SkipHint = FText::FromString(TEXT("按 ESC 跳过"));

	/** 是否正在播放 */
	UPROPERTY()
	bool bIsPlaying = false;
};