// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Input/DBAAndroidTouchInputBridge.h"
#include "Engine/World.h"

UDBAAndroidTouchInputBridge::UDBAAndroidTouchInputBridge()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Android 平台外不激活
#if !PLATFORM_ANDROID
	bAutoActivate = false;
#endif
}

void UDBAAndroidTouchInputBridge::BeginPlay()
{
	Super::BeginPlay();

	// Android 平台外不执行逻辑
#if !PLATFORM_ANDROID
	SetComponentTickEnabled(false);
#endif
}

void UDBAAndroidTouchInputBridge::OnSkillButtonLongPressStart(int32 SkillIndex, FVector2D TouchLocation)
{
	// 记录长按状态
	CurrentLongPressSkillIndex = SkillIndex;
	LongPressStartLocation = TouchLocation;
	bIsDragging = false;
	CurrentSkillDirection = FVector2D::ZeroVector;

	// 广播事件给 UI 层显示技能轮盘
	OnSkillWheelShowEvent.Broadcast(SkillIndex);
}

void UDBAAndroidTouchInputBridge::OnSkillButtonDrag(int32 SkillIndex, FVector2D DragDelta)
{
	if (CurrentLongPressSkillIndex != SkillIndex)
	{
		return;
	}

	bIsDragging = true;

	// 计算技能方向
	FVector2D NewLocation = LongPressStartLocation + DragDelta;
	CurrentSkillDirection = (NewLocation - LongPressStartLocation).GetSafeNormal();

	// 广播事件给 UI 层更新技能轮盘选中状态
	// 广播事件给 Targeting 系统更新技能方向指示器
	OnSkillDirectionUpdateEvent.Broadcast(CurrentSkillDirection);
}

void UDBAAndroidTouchInputBridge::OnSkillButtonRelease(int32 SkillIndex)
{
	if (CurrentLongPressSkillIndex != SkillIndex)
	{
		return;
	}

	// 如果正在拖拽，广播技能释放事件给 GAS 层
	if (bIsDragging)
	{
		OnSkillReleasedEvent.Broadcast(SkillIndex);
	}

	// 广播事件给 UI 层隐藏技能轮盘与指示器
	OnSkillWheelHideEvent.Broadcast();

	// 重置状态
	CurrentLongPressSkillIndex = -1;
	bIsDragging = false;
	CurrentSkillDirection = FVector2D::ZeroVector;
}

void UDBAAndroidTouchInputBridge::UpdateUltimateButtonState(float UltimateEnergy, bool bIsReady)
{
	// 此方法供 UI 层调用更新 Ultimate 按钮状态
	// UI 层根据 bIsReady 决定是否闪烁
	// 具体 UI 更新由蓝图实现
}
