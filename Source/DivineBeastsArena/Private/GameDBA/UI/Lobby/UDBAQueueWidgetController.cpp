// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/UDBAQueueWidgetController.h"
#include "GameDBA/Core/DBALogChannels.h"

/**
 * 构造函数
 * 初始化排队Widget控制器
 */
UDBAQueueWidgetController::UDBAQueueWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 请求加入队列
 * @param Mode 匹配模式
 */
void UDBAQueueWidgetController::RequestJoinQueue(int32 Mode)
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] 请求加入队列 - 模式: %d"), Mode);
}

/**
 * 请求离开队列
 */
void UDBAQueueWidgetController::RequestLeaveQueue()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] 请求离开队列"));
}

/**
 * 请求接受匹配
 */
void UDBAQueueWidgetController::RequestAcceptMatch()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] 请求接受匹配"));
}

/**
 * 请求拒绝匹配
 */
void UDBAQueueWidgetController::RequestDeclineMatch()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] 请求拒绝匹配"));
}
