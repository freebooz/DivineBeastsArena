// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Arena/DBAPlayerUnitFrameWidgetController.h"

/**
 * 构造函数
 * 初始化玩家单元框控制器
 */
UDBAPlayerUnitFrameWidgetController::UDBAPlayerUnitFrameWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 获取当前生命值
 * @return 当前 HP（待从 GAS 属性系统获取）
 */
float UDBAPlayerUnitFrameWidgetController::GetCurrentHP() const
{
	return 850.0f;
}

/**
 * 获取最大生命值
 * @return 最大 HP（待从 GAS 属性系统获取）
 */
float UDBAPlayerUnitFrameWidgetController::GetMaxHP() const
{
	return 1000.0f;
}

/**
 * 获取当前能量值
 * @return 当前能量（待从 GAS 属性系统获取）
 */
float UDBAPlayerUnitFrameWidgetController::GetCurrentEnergy() const
{
	return 70.0f;
}

/**
 * 获取最大能量值
 * @return 最大能量（待从 GAS 属性系统获取）
 */
float UDBAPlayerUnitFrameWidgetController::GetMaxEnergy() const
{
	return 100.0f;
}

/**
 * 获取当前等级
 * @return 英雄等级（待从 GAS 属性系统获取）
 */
int32 UDBAPlayerUnitFrameWidgetController::GetCurrentLevel() const
{
	return 12;
}
