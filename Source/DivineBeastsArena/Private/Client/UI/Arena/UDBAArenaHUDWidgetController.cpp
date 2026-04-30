// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Arena/DBAArenaHUDWidgetController.h"

/**
 * 构造函数
 * 初始化玩家属性默认值
 */
UDBAArenaHUDWidgetController::UDBAArenaHUDWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentHP(1000.0f)
	, MaxHP(1000.0f)
	, CurrentEnergy(100.0f)
	, MaxEnergy(100.0f)
{
}

/**
 * 更新玩家生命值
 * @param InCurrentHP 当前生命值
 * @param InMaxHP 最大生命值
 * 更新后广播 OnPlayerHPChanged 事件通知 UI 更新
 */
void UDBAArenaHUDWidgetController::UpdatePlayerHP(float InCurrentHP, float InMaxHP)
{
	CurrentHP = InCurrentHP;
	MaxHP = InMaxHP;
	OnPlayerHPChanged.Broadcast(CurrentHP, MaxHP);
}

/**
 * 更新玩家能量值
 * @param InCurrentEnergy 当前能量值
 * @param InMaxEnergy 最大能量值
 * 更新后广播 OnPlayerEnergyChanged 事件通知 UI 更新
 */
void UDBAArenaHUDWidgetController::UpdatePlayerEnergy(float InCurrentEnergy, float InMaxEnergy)
{
	CurrentEnergy = InCurrentEnergy;
	MaxEnergy = InMaxEnergy;
	OnPlayerEnergyChanged.Broadcast(CurrentEnergy, MaxEnergy);
}

/**
 * 更新终极能量值
 * @param InCurrentEnergy 当前终极能量值
 * @param InMaxEnergy 最大终极能量值（固定100）
 * 更新后广播 OnUltimateEnergyChanged 事件通知 UI 更新
 * 用于大招充能显示和就绪提示
 */
void UDBAArenaHUDWidgetController::UpdateUltimateEnergy(float InCurrentEnergy, float InMaxEnergy)
{
	OnUltimateEnergyChanged.Broadcast(InCurrentEnergy, InMaxEnergy);
}
