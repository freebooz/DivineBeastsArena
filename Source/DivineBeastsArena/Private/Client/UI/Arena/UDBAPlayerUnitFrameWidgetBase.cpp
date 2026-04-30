// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Arena/UDBAPlayerUnitFrameWidgetBase.h"

/**
 * 构造函数
 * 初始化玩家单元框 Widget
 * @param ObjectInitializer 对象初始化器
 */
UDBAPlayerUnitFrameWidgetBase::UDBAPlayerUnitFrameWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CachedCurrentHP(1000.0f)
	, CachedMaxHP(1000.0f)
	, CachedCurrentEnergy(100.0f)
	, CachedMaxEnergy(100.0f)
	, CurrentLevel(1)
{
}

/**
 * 原生构建回调
 * 当 Widget 构建到屏幕时调用
 */
void UDBAPlayerUnitFrameWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

/**
 * 原生销毁回调
 * 当 Widget 从屏幕移除时调用，用于清理
 */
void UDBAPlayerUnitFrameWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

/**
 * 原生 Tick 回调
 * 每帧更新玩家单元框状态
 * @param MyGeometry 当前 Widget 几何信息
 * @param InDeltaTime 帧间隔时间
 */
void UDBAPlayerUnitFrameWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

/**
 * Widget 被激活时的回调
 * 当玩家单元框显示时调用
 */
void UDBAPlayerUnitFrameWidgetBase::NativeOnActivated()
{
}

/**
 * Widget 被停用时的回调
 * 当玩家单元框隐藏时调用
 */
void UDBAPlayerUnitFrameWidgetBase::NativeOnDeactivated()
{
}

/**
 * 设置 Widget 控制器
 * 将控制器与 Widget 关联
 * @param InController 玩家单元框控制器指针
 */
void UDBAPlayerUnitFrameWidgetBase::SetWidgetController(UDBAPlayerUnitFrameWidgetController* InController)
{
	WidgetController = InController;
}

/**
 * 更新玩家生命值
 * 缓存当前值并计算血条百分比，通过 Blueprint 事件更新显示
 * @param InCachedCurrentHP 当前生命值
 * @param InCachedMaxHP 最大生命值
 */
void UDBAPlayerUnitFrameWidgetBase::UpdateHP(float InCachedCurrentHP, float InCachedMaxHP)
{
	CachedCurrentHP = InCachedCurrentHP;
	CachedMaxHP = InCachedMaxHP;

	// 计算血条百分比
	float Percentage = CachedMaxHP > 0.0f ? CachedCurrentHP / CachedMaxHP : 0.0f;
	BP_OnUpdateHP(CachedCurrentHP, CachedMaxHP, Percentage);
}

/**
 * 更新玩家能量值
 * 缓存当前值并计算能量条百分比，通过 Blueprint 事件更新显示
 * @param InCachedCurrentEnergy 当前能量值
 * @param InCachedMaxEnergy 最大能量值
 */
void UDBAPlayerUnitFrameWidgetBase::UpdateEnergy(float InCachedCurrentEnergy, float InCachedMaxEnergy)
{
	CachedCurrentEnergy = InCachedCurrentEnergy;
	CachedMaxEnergy = InCachedMaxEnergy;

	// 计算能量条百分比
	float Percentage = CachedMaxEnergy > 0.0f ? CachedCurrentEnergy / CachedMaxEnergy : 0.0f;
	BP_OnUpdateEnergy(CachedCurrentEnergy, CachedMaxEnergy, Percentage);
}

/**
 * 更新玩家等级
 * 缓存等级并通过 Blueprint 事件更新显示
 * @param Level 当前等级
 */
void UDBAPlayerUnitFrameWidgetBase::UpdateLevel(int32 Level)
{
	CurrentLevel = Level;
	BP_OnUpdateLevel(CurrentLevel);
}

/**
 * 应用五大阵营主题
 * 根据阵营更新玩家单元框的配色
 * @param FiveCamp 阵营类型（0-4 对应五大阵营）
 */
void UDBAPlayerUnitFrameWidgetBase::ApplyFiveCampTheme(uint8 FiveCamp)
{
	BP_OnApplyFiveCampTheme(FiveCamp);
}
