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
#include "Blueprint/IUserObjectListEntry.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UMG.h"
#include "UDBAStartupVideoWidget.generated.h"

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
	 * 视频播放完成时调用（由子类实现）
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "StartupVideo")
	void OnVideoFinishedBP();

	/**
	 * 跳过的点击事件（由子类实现）
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "StartupVideo")
	void OnSkipClickedBP();

	/**
	 * 视频播放完成内部处理
	 */
	void OnVideoFinished();

	/**
	 * 跳过按钮点击处理
	 */
	void OnSkipClicked();

private:
	/** 媒体播放器 */
	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> MediaPlayer;

	/** 跳过提示文本 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SkipHintText;

	/** 跳过按钮（透明全屏） */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SkipButton;

	/** 提示文字 */
	UPROPERTY(EditDefaultsOnly, Category = "StartupVideo")
	FText SkipHint = FText::FromString(TEXT("按 ESC 跳过"));

	/** 是否正在播放 */
	UPROPERTY(Transient)
	bool bIsPlaying = false;
};