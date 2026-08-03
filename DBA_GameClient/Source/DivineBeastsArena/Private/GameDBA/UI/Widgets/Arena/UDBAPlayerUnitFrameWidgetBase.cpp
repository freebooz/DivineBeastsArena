// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Widgets/Arena/UDBAPlayerUnitFrameWidgetBase.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/UI/Controllers/Arena/UDBAPlayerUnitFrameWidgetController.h"

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
	, CachedCurrentXP(0.0f)
	, CachedMaxXP(100.0f)
	, CachedUltimateEnergy(0.0f)
	, CachedMaxUltimateEnergy(DBAConstants::MaxUltimateEnergy)
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
	UpdateHP(CachedCurrentHP, CachedMaxHP);
	UpdateEnergy(CachedCurrentEnergy, CachedMaxEnergy);
	UpdateXP(CachedCurrentXP, CachedMaxXP);
	UpdateUltimateEnergyWithMax(CachedUltimateEnergy, CachedMaxUltimateEnergy);
	UpdateLevel(CurrentLevel);
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
	if (WidgetController)
	{
		WidgetController->OnHPUpdated.RemoveDynamic(this, &ThisClass::HandleControllerHPUpdated);
		WidgetController->OnEnergyUpdated.RemoveDynamic(this, &ThisClass::HandleControllerEnergyUpdated);
		WidgetController->OnLevelUpdated.RemoveDynamic(this, &ThisClass::HandleControllerLevelUpdated);
	}

	WidgetController = InController;

	if (!WidgetController)
	{
		return;
	}

	WidgetController->OnHPUpdated.AddDynamic(this, &ThisClass::HandleControllerHPUpdated);
	WidgetController->OnEnergyUpdated.AddDynamic(this, &ThisClass::HandleControllerEnergyUpdated);
	WidgetController->OnLevelUpdated.AddDynamic(this, &ThisClass::HandleControllerLevelUpdated);

	UpdateHP(WidgetController->GetCurrentHP(), WidgetController->GetMaxHP());
	UpdateEnergy(WidgetController->GetCurrentEnergy(), WidgetController->GetMaxEnergy());
	UpdateLevel(WidgetController->GetCurrentLevel());
}

void UDBAPlayerUnitFrameWidgetBase::HandleControllerHPUpdated(float CurrentHP, float MaxHP)
{
	UpdateHP(CurrentHP, MaxHP);
}

void UDBAPlayerUnitFrameWidgetBase::HandleControllerEnergyUpdated(float CurrentEnergy, float MaxEnergy)
{
	UpdateEnergy(CurrentEnergy, MaxEnergy);
}

void UDBAPlayerUnitFrameWidgetBase::HandleControllerLevelUpdated(int32 Level)
{
	UpdateLevel(Level);
}

/**
 * 更新玩家生命值
 * 缓存当前值并计算血条百分比，通过 Blueprint 事件更新显示
 * @param InCachedCurrentHP 当前生命值
 * @param InCachedMaxHP 最大生命值
 */
void UDBAPlayerUnitFrameWidgetBase::UpdateHP(float InCachedCurrentHP, float InCachedMaxHP)
{
	CachedCurrentHP = FMath::Max(0.0f, InCachedCurrentHP);
	CachedMaxHP = FMath::Max(0.0f, InCachedMaxHP);

	// 计算血条百分比
	float Percentage = FMath::Clamp(CachedMaxHP > 0.0f ? CachedCurrentHP / CachedMaxHP : 0.0f, 0.0f, 1.0f);
	BP_OnUpdateHP(CachedCurrentHP, CachedMaxHP, Percentage);

	// 更新生命条 Widget
	if (HealthBar)
	{
		HealthBar->SetPercent(Percentage);
	}
}

/**
 * 更新玩家能量值
 * 缓存当前值并计算能量条百分比，通过 Blueprint 事件更新显示
 * @param InCachedCurrentEnergy 当前能量值
 * @param InCachedMaxEnergy 最大能量值
 */
void UDBAPlayerUnitFrameWidgetBase::UpdateEnergy(float InCachedCurrentEnergy, float InCachedMaxEnergy)
{
	CachedCurrentEnergy = FMath::Max(0.0f, InCachedCurrentEnergy);
	CachedMaxEnergy = FMath::Max(0.0f, InCachedMaxEnergy);

	// 计算能量条百分比
	float Percentage = FMath::Clamp(CachedMaxEnergy > 0.0f ? CachedCurrentEnergy / CachedMaxEnergy : 0.0f, 0.0f, 1.0f);
	BP_OnUpdateEnergy(CachedCurrentEnergy, CachedMaxEnergy, Percentage);

	// 更新能量条 Widget
	if (EnergyBar)
	{
		EnergyBar->SetPercent(Percentage);
	}
}

void UDBAPlayerUnitFrameWidgetBase::UpdateXP(float InCachedCurrentXP, float InCachedMaxXP)
{
	CachedCurrentXP = FMath::Max(0.0f, InCachedCurrentXP);
	CachedMaxXP = FMath::Max(0.0f, InCachedMaxXP);

	// 计算经验条百分比
	float Percentage = FMath::Clamp(CachedMaxXP > 0.0f ? CachedCurrentXP / CachedMaxXP : 0.0f, 0.0f, 1.0f);
	BP_OnUpdateXP(CachedCurrentXP, CachedMaxXP, Percentage);

	// 更新经验条 Widget
	if (XPBar)
	{
		XPBar->SetPercent(Percentage);
	}
}

void UDBAPlayerUnitFrameWidgetBase::UpdateUltimateEnergy(float Energy)
{
	UpdateUltimateEnergyWithMax(Energy, DBAConstants::MaxUltimateEnergy);
}

void UDBAPlayerUnitFrameWidgetBase::UpdateUltimateEnergyWithMax(float Energy, float MaxEnergy)
{
	CachedMaxUltimateEnergy = FMath::Max(1.0f, MaxEnergy);
	CachedUltimateEnergy = FMath::Clamp(Energy, 0.0f, CachedMaxUltimateEnergy);

	float Percentage = CachedUltimateEnergy / CachedMaxUltimateEnergy;
	BP_OnUpdateUltimateEnergy(CachedUltimateEnergy, Percentage);

	// 更新终极能量条 Widget
	if (UltimateEnergyBar)
	{
		UltimateEnergyBar->SetPercent(Percentage);
	}
}

void UDBAPlayerUnitFrameWidgetBase::UpdateLevel(int32 Level)
{
	CurrentLevel = FMath::Max(1, Level);
	BP_OnUpdateLevel(CurrentLevel);
}

void UDBAPlayerUnitFrameWidgetBase::ApplyFiveCampTheme(uint8 FiveCamp)
{
	const uint8 NormalizedFiveCamp = FMath::Clamp(
		FiveCamp,
		static_cast<uint8>(EDBAFiveCamp::None),
		static_cast<uint8>(EDBAFiveCamp::Center));

	BP_OnApplyFiveCampTheme(NormalizedFiveCamp);
}
