// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Data/DBADataAssetBase.h"
#include "DBALobbyInputConfigDataAsset.generated.h"

class UInputAction;

/**
 * 大厅输入配置数据资产
 *
 * P1-4 改造：将大厅玩家控制器的所有 UInputAction 软引用集中配置，
 * 避免在 C++ 中硬编码输入资产路径（符合 DBA.DataAsset.NoHardcoding 策略）。
 * P1-1 改造：统一继承 UDBADataAssetBase，复用通用数据资产基类字段。
 *
 * 配置项分组：
 * - Movement：移动与视角控制（Axis1D）
 * - Combat：技能释放（Boolean）
 * - UI：界面切换（Boolean）
 * - Mouse：鼠标交互（Boolean）
 *
 * 设计依据：
 *   - 项目策略《DBA.DataAsset.NoHardcoding》：输入配置通过 DataAsset 驱动
 *   - 项目策略《DBA.UI.EventAsync》：输入事件由 Enhanced Input 系统驱动
 *   - 审查报告 P1-4/P1-7：输入配置未数据资产化
 *   - 审查报告 P1-1：数据资产体系统一
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBALobbyInputConfigDataAsset : public UDBADataAssetBase
{
	GENERATED_BODY()

public:
	// ==================== 移动与视角（Axis1D） ====================

	/** 前进/后退输入动作（W/S 映射，Axis1D） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Movement")
	TSoftObjectPtr<UInputAction> MoveForward;

	/** 左移/右移输入动作（A/D 映射，Axis1D） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Movement")
	TSoftObjectPtr<UInputAction> MoveRight;

	/** 水平视角转动输入动作（鼠标 X 轴，Axis1D） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Movement")
	TSoftObjectPtr<UInputAction> Turn;

	/** 垂直视角转动输入动作（鼠标 Y 轴，Axis1D） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Movement")
	TSoftObjectPtr<UInputAction> LookUp;

	// ==================== 战斗技能（Boolean） ====================

	/** 技能 01 输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Combat")
	TSoftObjectPtr<UInputAction> Skill01;

	/** 技能 02 输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Combat")
	TSoftObjectPtr<UInputAction> Skill02;

	/** 技能 03 输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Combat")
	TSoftObjectPtr<UInputAction> Skill03;

	/** 技能 04 输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Combat")
	TSoftObjectPtr<UInputAction> Skill04;

	/** 终极技能输入动作（PC 默认 R → Ultimate） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Combat")
	TSoftObjectPtr<UInputAction> Ultimate;

	/** 普通攻击输入动作（PC 默认鼠标左键 → BasicAttack） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Combat")
	TSoftObjectPtr<UInputAction> BasicAttack;

	/** 闪避输入动作（PC 默认 Space；GAS Dodge 能力接入前仅记录绑定入口） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Combat")
	TSoftObjectPtr<UInputAction> Dodge;

	/** 切换锁定目标输入动作（PC 默认 Tab → CycleLockTarget） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Combat")
	TSoftObjectPtr<UInputAction> CycleLockTarget;

	/** 技能 06 输入动作（遗留/扩展槽，非 V2 核心战斗输入） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Combat")
	TSoftObjectPtr<UInputAction> Skill06;

	// ==================== UI 交互（Boolean） ====================

	/** 背包界面切换输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|UI")
	TSoftObjectPtr<UInputAction> Inventory;

	/** ESC 菜单输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|UI")
	TSoftObjectPtr<UInputAction> Escape;

	// ==================== 鼠标交互（Boolean） ====================

	/** 鼠标左键输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Mouse")
	TSoftObjectPtr<UInputAction> LeftMouse;

	/** 鼠标右键输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Mouse")
	TSoftObjectPtr<UInputAction> RightMouse;

	/** 鼠标滚轮向上输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Mouse")
	TSoftObjectPtr<UInputAction> ScrollUp;

	/** 鼠标滚轮向下输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Mouse")
	TSoftObjectPtr<UInputAction> ScrollDown;
};
