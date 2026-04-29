// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Arena/DBAArenaHUDRootWidgetBase.h"

/**
 * 构造函数
 * 初始化 HUD 根 Widget
 * @param ObjectInitializer 对象初始化器
 */
UDBAArenaHUDRootWidgetBase::UDBAArenaHUDRootWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsEditMode(false)
{
}

/**
 * 原生构建回调
 * 当 Widget 构建到屏幕时调用，进行事件绑定等初始化操作
 */
void UDBAArenaHUDRootWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

/**
 * 原生销毁回调
 * 当 Widget 从屏幕移除时调用，进行事件解绑等清理操作
 */
void UDBAArenaHUDRootWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

/**
 * 原生 Tick 回调
 * 每帧更新 HUD 状态
 * @param MyGeometry 当前 Widget 几何信息
 * @param InDeltaTime 帧间隔时间
 */
void UDBAArenaHUDRootWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

/**
 * Widget 被激活时的回调
 * 当 HUD 显示时调用，可重写以执行显示逻辑
 */
void UDBAArenaHUDRootWidgetBase::NativeOnActivated()
{
}

/**
 * Widget 被停用时的回调
 * 当 HUD 隐藏时调用，可重写以执行隐藏逻辑
 */
void UDBAArenaHUDRootWidgetBase::NativeOnDeactivated()
{
}

/**
 * 设置 Widget 控制器
 * 将控制器与 Widget 关联，使 Widget 可以接收控制器更新的数据
 * @param InController HUD Widget 控制器指针
 */
void UDBAArenaHUDRootWidgetBase::SetWidgetController(UDBAArenaHUDWidgetController* InController)
{
	WidgetController = InController;
}

/**
 * 设置 HUD 可见性
 * @param bVisible true 显示 HUD，false 隐藏 HUD
 */
void UDBAArenaHUDRootWidgetBase::SetHUDVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

/**
 * 设置 HUD 编辑模式
 * @param bEditMode true 编辑模式，false 普通模式
 * 编辑模式下可能显示额外调试信息
 */
void UDBAArenaHUDRootWidgetBase::SetHUDEditMode(bool bEditMode)
{
	bIsEditMode = bEditMode;
}

/**
 * 应用五大阵营主题
 * 根据选择的阵营改变 HUD 的配色和样式
 * @param FiveCamp 阵营类型（0-4 对应五大阵营）
 */
void UDBAArenaHUDRootWidgetBase::ApplyFiveCampTheme(uint8 FiveCamp)
{
	BP_OnApplyFiveCampTheme(FiveCamp);
}
