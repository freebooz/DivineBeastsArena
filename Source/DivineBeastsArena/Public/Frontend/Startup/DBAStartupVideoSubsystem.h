// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Subsystems/DBASubsystemImpl.h"
#include "TimerManager.h"
#include "DBAStartupVideoSubsystem.generated.h"

class UMediaPlayer;
class UMediaTexture;
class UUserWidget;

/**
 * 启动视频状态
 */
UENUM(BlueprintType)
enum class EDBAStartupVideoState : uint8
{
	NotStarted,      // 未开始
	Playing,         // 播放中
	Skipped,         // 已跳过
	Finished         // 播放完成
};

/**
 * DBAStartupVideoSubsystem
 *
 * 管理游戏启动视频播放的子系统
 * 功能：
 * - 播放启动视频（UE5 MediaFramework）
 * - ESC 按键跳过视频
 * - 播放完成后切换到登录界面
 * - 支持客户端/服务器环境（仅客户端播放）
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAStartupVideoSubsystem : public UGameInstanceSubsystem, public DBASubsystemImpl
{
	GENERATED_BODY()

public:
	UDBAStartupVideoSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	// End USubsystem interface

	/**
	 * 开始播放启动视频
	 */
	void PlayStartupVideo();

	/**
	 * 跳过启动视频
	 */
	void SkipVideo();

	/**
	 * 获取当前状态
	 */
	EDBAStartupVideoState GetState() const { return CurrentState; }

	/**
	 * 启动视频播放完成事件
	 * 视频播放完成或跳过时触发
	 * 用于连接前端系统加载登录界面
	 */
	DECLARE_EVENT_OneParam(UDBAStartupVideoSubsystem, FDBAOnStartupVideoFinished, EDBAStartupVideoState /* FinishedState */)

	/**
	 * 订阅启动视频完成事件
	 */
	FDBAOnStartupVideoFinished& OnStartupVideoFinished() { return StartupVideoFinishedEvent; }

public:
	/** 启动视频完成事件广播 */
	FDBAOnStartupVideoFinished StartupVideoFinishedEvent;

protected:
	/**
	 * 创建视频播放 UI
	 */
	void CreateVideoWidget();

	/**
	 * 视频播放完成回调
	 */
	void OnVideoFinished();

	/**
	 * 处理 ESC 输入
	 */
	void HandleESCInput();

	/**
	 * 跳转到登录界面
	 */
	void NavigateToLoginScreen();

private:
	/** 视频播放器 */
	UPROPERTY()
	TObjectPtr<UMediaPlayer> MediaPlayer;

	/** 视频纹理 */
	UPROPERTY()
	TObjectPtr<UMediaTexture> MediaTexture;

	/** 视频播放控件 */
	UPROPERTY()
	TObjectPtr<UUserWidget> VideoWidget;

	/** 当前状态 */
	UPROPERTY()
	EDBAStartupVideoState CurrentState = EDBAStartupVideoState::NotStarted;

	/** 是否已绑定 ESC 输入 */
	bool bESCBound = false;

	/** 视频播放完成定时器句柄 */
	FTimerHandle VideoFinishedTimerHandle;

	/** 视频路径（可以在子类或 DataAsset 中配置） */
	UPROPERTY(EditDefaultsOnly, Category = "StartupVideo")
	FString StartupVideoPath = TEXT("/Engine/Movies/DefaultCluster.mp4");

	/** 视频播放超时时间（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "StartupVideo")
	float VideoTimeout = 60.0f;
};