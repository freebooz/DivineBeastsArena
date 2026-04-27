// Copyright Epic Games, Inc. All Rights Reserved.

#include "Common/UI/DBAWidgetController.h"

/**
 * 构造函数
 * 初始化 Widget 控制器基类
 */
UDBAWidgetController::UDBAWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 初始化控制器
 * 设置 bIsInitialized 为 true，表示控制器已准备好
 * 由持有控制器的 Widget 或Subsystem 调用
 */
void UDBAWidgetController::InitializeController()
{
	bIsInitialized = true;
}

/**
 * 重置控制器状态
 * 设置 bIsInitialized 为 false，表示控制器已失效
 * 用于 Widget 销毁或控制器切换时清理状态
 */
void UDBAWidgetController::ResetController()
{
	bIsInitialized = false;
}
