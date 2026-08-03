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
#include "Engine/DeveloperSettings.h"
#include "DBAEnhancedInputDeveloperSettings.generated.h"

class UDBALobbyInputConfigDataAsset;
class UDBASpectatorInputConfigDataAsset;
class UInputMappingContext;

/**
 * DBA Enhanced Input 开发者设置
 *
 * P1-4 改造：配置默认输入配置数据资产和 Input Mapping Context，
 * 避免在 C++ 中硬编码输入资产路径（符合 DBA.DataAsset.NoHardcoding 策略）。
 *
 * 配置项：
 * - DefaultLobbyInputConfig：大厅输入配置数据资产
 * - DefaultLobbyInputMappingContext：大厅 Input Mapping Context 资产
 * - DefaultSpectatorInputConfig：观战输入配置数据资产
 * - DefaultSpectatorInputMappingContext：观战 Input Mapping Context 资产
 *
 * 设计依据：
 *   - 项目策略《DBA.DataAsset.NoHardcoding》：输入配置通过 DeveloperSettings 驱动
 *   - 审查报告 P1-5：缺 IMC 资产；P1-7：输入配置未数据资产化
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "DBA Enhanced Input 设置"))
class DIVINEBEASTSARENA_API UDBAEnhancedInputDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 大厅默认输入配置数据资产 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Lobby", meta = (AllowedClasses = "/Script/DivineBeastsArena.DBAInputConfigDataAsset"))
	TSoftObjectPtr<UDBALobbyInputConfigDataAsset> DefaultLobbyInputConfig;

	/** 大厅默认 Input Mapping Context */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Lobby")
	TSoftObjectPtr<UInputMappingContext> DefaultLobbyInputMappingContext;

	/**
	 * 竞技场默认输入配置数据资产（复用 UDBALobbyInputConfigDataAsset；战斗键位 Q/W/E/R 在 IMC_Arena 中映射）
	 * 编辑器占位：/Game/DBA/Input/DA_ArenaInputConfig.DA_ArenaInputConfig
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Arena", meta = (AllowedClasses = "/Script/DivineBeastsArena.DBALobbyInputConfigDataAsset"))
	TSoftObjectPtr<UDBALobbyInputConfigDataAsset> DefaultArenaInputConfig;

	/**
	 * 竞技场默认 Input Mapping Context
	 * 编辑器占位：/Game/DBA/Input/IMC_Arena.IMC_Arena
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Arena")
	TSoftObjectPtr<UInputMappingContext> DefaultArenaInputMappingContext;

	/** 观战默认输入配置数据资产 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Spectator", meta = (AllowedClasses = "/Script/DivineBeastsArena.DBAInputConfigDataAsset"))
	TSoftObjectPtr<UDBASpectatorInputConfigDataAsset> DefaultSpectatorInputConfig;

	/** 观战默认 Input Mapping Context */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|Input|Spectator")
	TSoftObjectPtr<UInputMappingContext> DefaultSpectatorInputMappingContext;
};
