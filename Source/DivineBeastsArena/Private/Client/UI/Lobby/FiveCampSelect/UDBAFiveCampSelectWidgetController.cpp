// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/FiveCampSelect/DBAFiveCampSelectWidgetController.h"

/**
 * 构造函数
 * 初始化阵营选择 Widget 控制器
 */
UDBAFiveCampSelectWidgetController::UDBAFiveCampSelectWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 确认阵营选择
 * 用户选择阵营后调用此方法确认选择
 * @param FiveCamp 选择的阵营类型
 */
void UDBAFiveCampSelectWidgetController::ConfirmFiveCampSelection(EDBAFiveCamp FiveCamp)
{
	HandleFiveCampConfirmed(true, FiveCamp);
}

/**
 * 请求返回上一页
 * 用于取消当前选择并返回上一界面
 */
void UDBAFiveCampSelectWidgetController::RequestBack()
{
}

/**
 * 处理阵营确认回调
 * @param bSuccess 是否成功确认
 * @param FiveCamp 确认的阵营类型
 * 成功时广播 OnFiveCampConfirmed 事件
 */
void UDBAFiveCampSelectWidgetController::HandleFiveCampConfirmed(bool bSuccess, EDBAFiveCamp FiveCamp)
{
	if (bSuccess)
	{
		OnFiveCampConfirmed.Broadcast(FiveCamp);
	}
}
