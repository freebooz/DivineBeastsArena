// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/Loading/DBALoadingWidgetController.h"

/**
 * 构造函数
 * 初始化加载界面 Widget 控制器
 */
UDBALoadingWidgetController::UDBALoadingWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LoadingProgress(0.0f)
{
}

/**
 * 请求加载完成
 * 当资源加载完成时调用此方法通知 UI 隐藏加载界面
 * 广播 OnLoadingComplete 事件
 */
void UDBALoadingWidgetController::RequestLoadComplete()
{
	OnLoadingComplete.Broadcast();
}

/**
 * 获取加载进度
 * @return 当前加载进度（0.0 - 1.0）
 */
float UDBALoadingWidgetController::GetLoadingProgress() const
{
	return LoadingProgress;
}
