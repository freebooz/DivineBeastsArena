// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/UDBAQueueWidgetController.h"
#include "GameDBA/Core/DBALogChannels.h"

/**
 * 鏋勯€犲嚱鏁? * 鍒濆鍖栨帓闃?Widget 鎺у埗鍣? */
UDBAQueueWidgetController::UDBAQueueWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 璇锋眰鍔犲叆闃熷垪
 * @param Mode 鎺掗槦妯″紡
 */
void UDBAQueueWidgetController::RequestJoinQueue(int32 Mode)
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] RequestJoinQueue - Mode: %d"), Mode);
}

/**
 * 璇锋眰绂诲紑闃熷垪
 */
void UDBAQueueWidgetController::RequestLeaveQueue()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] RequestLeaveQueue"));
}

/**
 * 璇锋眰鎺ュ彈鍖归厤
 */
void UDBAQueueWidgetController::RequestAcceptMatch()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] RequestAcceptMatch"));
}

/**
 * 璇锋眰鎷掔粷鍖归厤
 */
void UDBAQueueWidgetController::RequestDeclineMatch()
{
	UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueWidgetController] RequestDeclineMatch"));
}

