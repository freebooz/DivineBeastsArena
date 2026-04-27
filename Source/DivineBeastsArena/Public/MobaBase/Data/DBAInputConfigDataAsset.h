// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MobaBase/Input/DBAInputActionSetTypes.h"
#include "MobaBase/Input/DBAInputMappingContextTypes.h"
#include "DBAInputConfigDataAsset.generated.h"

/**
 * 输入配置数据资产
 * 集中管理所有输入动作与映射上下文
 * 支持 PC 与 Android 平台差异化配置
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAInputConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 核心战斗输入动作集（固定 6 个战斗输入） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions", meta = (DisplayName = "核心战斗输入"))
	FDBACombatInputActionSet CombatActions;

	/** 系统输入动作集（锁定目标、Ping、计分板、菜单、聊天、地图、交互） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions", meta = (DisplayName = "系统输入"))
	FDBASystemInputActionSet SystemActions;

	/** 移动与摄像机输入动作集 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Actions", meta = (DisplayName = "移动输入"))
	FDBAMovementInputActionSet MovementActions;

	/** 输入映射上下文配置列表（支持多平台） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mapping Contexts", meta = (DisplayName = "映射上下文列表"))
	TArray<FDBAInputMappingContextConfig> MappingContexts;

	/**
	 * 根据平台类型获取对应的映射上下文配置
	 * @param Platform 目标平台类型
	 * @param OutConfigs 输出的映射上下文配置列表
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Config")
	void GetMappingContextsForPlatform(EDBAInputPlatform Platform, TArray<FDBAInputMappingContextConfig>& OutConfigs) const;
};
