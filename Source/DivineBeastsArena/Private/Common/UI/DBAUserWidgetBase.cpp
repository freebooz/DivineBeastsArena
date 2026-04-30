// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Common/UI/DBAUserWidgetBase.h"

/**
 * 构造函数
 * 初始化用户 Widget 基类
 */
UDBAUserWidgetBase::UDBAUserWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 原生初始化回调
 * 当 Widget 控制器和绑定Widget 完成初始化时调用
 */
void UDBAUserWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

/**
 * 原生构建回调
 * 当 Widget 被构建到屏幕上时调用
 * 适合进行事件绑定、数据初始化等操作
 */
void UDBAUserWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

/**
 * 原生销毁回调
 * 当 Widget 从屏幕上移除时调用
 * 适合进行事件解绑、计时器清理等操作
 */
void UDBAUserWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

/**
 * 原生 Tick 回调
 * 每帧调用，用于 Widget 动画和状态更新
 * @param MyGeometry 当前 Widget 的几何信息
 * @param InDeltaTime 距离上一帧的时间
 */
void UDBAUserWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

/**
 * 激活 Widget
 * 设置 bIsActive 为 true，表示 Widget 正在显示
 * 子类可重写此方法添加激活时的逻辑
 */
void UDBAUserWidgetBase::Activate()
{
	bIsActive = true;
}

/**
 * 停用 Widget
 * 设置 bIsActive 为 false，表示 Widget 已隐藏
 * 子类可重写此方法添加停用时的逻辑
 */
void UDBAUserWidgetBase::Deactivate()
{
	bIsActive = false;
}
