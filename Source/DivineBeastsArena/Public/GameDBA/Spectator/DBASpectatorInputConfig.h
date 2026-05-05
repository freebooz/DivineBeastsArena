// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DBASpectatorInputConfig.generated.h"

/**
 * UDBASpectatorInputConfig
 * 观战系统输入配置
 * 用于在运行时自动配置观战模式的输入映射
 * 在 UE 编辑器中编译后，需要在项目设置中启用此配置
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBASpectatorInputConfig : public UObject
{
	GENERATED_BODY()

public:
	UDBASpectatorInputConfig();

	/** 配置观战输入 */
	static void ConfigureSpectatorInput(UWorld* World);

	/** 获取观战输入映射上下文类名 */
	static FString GetInputMappingContextName() { return TEXT("IMC_Spectator"); }

	/** 获取观战输入优先级 */
	static int32 GetInputPriority() { return 100; }
};
