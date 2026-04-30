// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/DBAQueueWidgetController.h"

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
 * @param Mode 排队模式（快速匹配、排位赛、自定义房间）
 * 调用 QueueSubsystem 的 JoinQueue 方法加入排队
 */
void UDBAQueueWidgetController::RequestJoinQueue(EDBAQueueSubsystemMode Mode)
{
}

/**
 * 请求离开队列
 * 调用 QueueSubsystem 的 LeaveQueue 方法离开排队
 */
void UDBAQueueWidgetController::RequestLeaveQueue()
{
}

/**
 * 请求接受匹配
 * 当匹配成功时调用此方法确认接受
 */
void UDBAQueueWidgetController::RequestAcceptMatch()
{
}

/**
 * 请求拒绝匹配
 * 当匹配成功时调用此方法拒绝并返回排队
 */
void UDBAQueueWidgetController::RequestDeclineMatch()
{
}
