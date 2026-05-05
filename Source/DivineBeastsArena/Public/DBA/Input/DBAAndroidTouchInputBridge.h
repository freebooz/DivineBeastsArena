// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DBAAndroidTouchInputBridge.generated.h"

class UInputAction;

/**
 * Android 触屏输入桥接组件
 * 处理 Android 平台特有的触屏输入逻辑：
 * - 技能轮盘（长按展开、拖拽选择、松手释放）
 * - Ultimate 大按钮（能量满闪烁、冷却遮罩）
 * - 触屏目标选择（拖拽指示器）
 *
 * 本组件仅在 Android 平台激活，PC 平台不创建
 */
UCLASS(ClassGroup = (Input), meta = (BlueprintSpawnableComponent, DisplayName = "Android 触屏输入桥接"))
class DIVINEBEASTSARENA_API UDBAAndroidTouchInputBridge : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBAAndroidTouchInputBridge();

protected:
	virtual void BeginPlay() override;

public:
	/**
	 * 技能轮盘展开事件
	 * 广播给 UI 层显示技能轮盘
	 */
	DECLARE_EVENT_OneParam(UDBAAndroidTouchInputBridge, FDBAOnSkillWheelShow, int32 /* SkillIndex */)

	/**
	 * 技能轮盘隐藏事件
	 * 广播给 UI 层隐藏技能轮盘
	 */
	DECLARE_EVENT(UDBAAndroidTouchInputBridge, FDBAOnSkillWheelHide)

	/**
	 * 技能方向更新事件
	 * 广播给 Targeting 系统更新技能方向指示器
	 */
	DECLARE_EVENT_OneParam(UDBAAndroidTouchInputBridge, FDBAOnSkillDirectionUpdate, FVector2D /* Direction */)

	/**
	 * 技能释放事件
	 * 广播给 GAS 层触发技能释放
	 */
	DECLARE_EVENT_OneParam(UDBAAndroidTouchInputBridge, FDBAOnSkillReleased, int32 /* SkillIndex */)

	/**
	 * 技能轮盘展开事件广播
	 */
	FDBAOnSkillWheelShow OnSkillWheelShowEvent;

	/**
	 * 技能轮盘隐藏事件广播
	 */
	FDBAOnSkillWheelHide OnSkillWheelHideEvent;

	/**
	 * 技能方向更新事件广播
	 */
	FDBAOnSkillDirectionUpdate OnSkillDirectionUpdateEvent;

	/**
	 * 技能释放事件广播
	 */
	FDBAOnSkillReleased OnSkillReleasedEvent;

	/**
	 * 处理技能按钮长按开始
	 * @param SkillIndex 技能索引（0=BasicAttack, 1~4=Skill01~04, 5=Ultimate）
	 * @param TouchLocation 触摸屏幕位置
	 */
	UFUNCTION(BlueprintCallable, Category = "Android Touch Input")
	void OnSkillButtonLongPressStart(int32 SkillIndex, FVector2D TouchLocation);

	/**
	 * 处理技能按钮拖拽
	 * @param SkillIndex 技能索引
	 * @param DragDelta 拖拽增量
	 */
	UFUNCTION(BlueprintCallable, Category = "Android Touch Input")
	void OnSkillButtonDrag(int32 SkillIndex, FVector2D DragDelta);

	/**
	 * 处理技能按钮松手释放
	 * @param SkillIndex 技能索引
	 */
	UFUNCTION(BlueprintCallable, Category = "Android Touch Input")
	void OnSkillButtonRelease(int32 SkillIndex);

	/**
	 * 更新 Ultimate 按钮状态
	 * @param UltimateEnergy 当前终极能量值（0~100）
	 * @param bIsReady 是否可释放
	 */
	UFUNCTION(BlueprintCallable, Category = "Android Touch Input")
	void UpdateUltimateButtonState(float UltimateEnergy, bool bIsReady);

private:
	/** 当前长按的技能索引（-1 表示无） */
	int32 CurrentLongPressSkillIndex = -1;

	/** 长按起始位置 */
	FVector2D LongPressStartLocation = FVector2D::ZeroVector;

	/** 是否正在拖拽 */
	bool bIsDragging = false;

	/** 当前技能方向 */
	FVector2D CurrentSkillDirection = FVector2D::ZeroVector;
};
