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
#include "DBASpectatorInputConfigDataAsset.generated.h"

class UInputAction;

/**
 * 观战输入配置数据资产
 *
 * P1-4 改造：将观战组件的所有 UInputAction 软引用集中配置，
 * 避免在 C++ 中硬编码输入资产路径（符合 DBA.DataAsset.NoHardcoding 策略）。
 * P1-1 改造：统一继承 UDBADataAssetBase，复用通用数据资产基类字段。
 *
 * 设计依据：
 *   - 项目策略《DBA.DataAsset.NoHardcoding》：输入配置通过 DataAsset 驱动
 *   - 项目策略《DBA.UI.EventAsync》：输入事件由 Enhanced Input 系统驱动
 *   - 审查报告 P1-4/P1-7：输入配置未数据资产化
 *   - 审查报告 P1-1：数据资产体系统一
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBASpectatorInputConfigDataAsset : public UDBADataAssetBase
{
	GENERATED_BODY()

public:
	/** 切换到下一个观战目标 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Spectator")
	TSoftObjectPtr<UInputAction> CycleNext;

	/** 切换到上一个观战目标 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Spectator")
	TSoftObjectPtr<UInputAction> CyclePrevious;

	/** 切换自由视角/跟随视角 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Spectator")
	TSoftObjectPtr<UInputAction> ToggleFreeView;

	/** 暂停/恢复观战 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Spectator")
	TSoftObjectPtr<UInputAction> TogglePause;

	/**
	 * 数字键快捷切换输入动作数组
	 * 索引 0~8 分别对应数字键 1~9
	 * 在 Input Mapping Context 中为每个元素绑定对应数字键
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Spectator")
	TArray<TSoftObjectPtr<UInputAction>> NumericSwitchActions;
};
