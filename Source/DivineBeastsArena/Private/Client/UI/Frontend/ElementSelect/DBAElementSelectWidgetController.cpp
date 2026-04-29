// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Frontend/ElementSelect/DBAElementSelectWidgetController.h"

/**
 * 构造函数
 * 初始化元素选择 Widget 控制器
 */
UDBAElementSelectWidgetController::UDBAElementSelectWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 确认元素选择
 * 用户选择元素后调用此方法确认选择
 * @param Element 选择的元素类型
 */
void UDBAElementSelectWidgetController::ConfirmElementSelection(EDBAElement Element)
{
	HandleElementConfirmed(true, Element);
}

/**
 * 请求返回上一页
 * 用于取消当前选择并返回上一界面
 */
void UDBAElementSelectWidgetController::RequestBack()
{
}

/**
 * 处理元素确认回调
 * @param bSuccess 是否成功确认
 * @param Element 确认的元素类型
 * 成功时广播 OnElementConfirmed 事件
 */
void UDBAElementSelectWidgetController::HandleElementConfirmed(bool bSuccess, EDBAElement Element)
{
	if (bSuccess)
	{
		OnElementConfirmed.Broadcast(Element);
	}
}
