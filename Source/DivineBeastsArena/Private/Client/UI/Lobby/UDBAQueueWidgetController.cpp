// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/UDBAQueueWidgetController.h"
#include "Common/DBALogChannels.h"

/**
 * 构造函数
 * 初始化排队 Widget 控制器
 */
UDBAQueueWidgetController::UDBAQueueWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 请求加入队列
 * @param Mode 排队模式
 */
void UDBAQueueWidgetController::RequestJoinQueue(int32 Mode)
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] RequestJoinQueue - Mode: %d"), Mode);
}

/**
 * 请求离开队列
 */
void UDBAQueueWidgetController::RequestLeaveQueue()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] RequestLeaveQueue"));
}

/**
 * 请求接受匹配
 */
void UDBAQueueWidgetController::RequestAcceptMatch()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] RequestAcceptMatch"));
}

/**
 * 请求拒绝匹配
 */
void UDBAQueueWidgetController::RequestDeclineMatch()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] RequestDeclineMatch"));
}
