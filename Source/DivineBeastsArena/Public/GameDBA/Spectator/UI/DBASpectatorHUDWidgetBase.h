// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/UI/DBAUserWidgetBase.h"
#include "GameDBA/Spectator/DBAObserverTypes.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "DBASpectatorHUDWidgetBase.generated.h"

class UDBASpectatorComponent;
class UDBASpectatorStatusBarWidgetBase;
class UDBASpectatorMinimapWidgetBase;

/**
 * UDBASpectatorHUDWidgetBase
 * 观战HUD Widget基类
 * 包含状态栏、小地图、控制面板
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBASpectatorHUDWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBASpectatorHUDWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	/** 绑定观战组件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|HUD")
	void BindSpectatorComponent(UDBASpectatorComponent* InSpectatorComponent);

	/** 解绑观战组件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|HUD")
	void UnbindSpectatorComponent();

protected:
	/** 更新视角目标显示 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Spectator|HUD")
	void OnViewTargetUpdated(const FDBAObserverViewTarget& ViewTarget);

	/** 更新暂停状态 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Spectator|HUD")
	void OnPauseStateChanged(bool bIsPaused);

	/** 更新观战者列表 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Spectator|HUD")
	void OnObserverListUpdated(const TArray<FDBAObserverViewTarget>& Targets);

protected:
	/** 顶部状态栏 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDBASpectatorStatusBarWidgetBase> StatusBar;

	/** 小地图 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UDBASpectatorMinimapWidgetBase> Minimap;

	/** 当前观看的玩家名称 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentPlayerNameText;

	/** 暂停提示文本 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PauseIndicatorText;

	/** 暂停状态覆盖层 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UWidget* PauseOverlay;

protected:
	/** 观战组件引用 */
	UPROPERTY(Transient)
	TWeakObjectPtr<UDBASpectatorComponent> SpectatorComponent;

	/** 缓存的当前视角目标 */
	UPROPERTY(Transient)
	FDBAObserverViewTarget CachedViewTarget;

	/** 缓存的暂停状态 */
	UPROPERTY(Transient)
	bool bCachedIsPaused;
};
