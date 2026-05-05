// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "DBAInputMappingContextTypes.generated.h"

/**
 * 输入映射上下文平台类型
 * 用于区分 PC 与 Android 的输入映射差异
 */
UENUM(BlueprintType)
enum class EDBAInputPlatform : uint8
{
	/** PC 平台（键鼠） */
	PC UMETA(DisplayName = "PC"),

	/** Android 平台（触屏） */
	Android UMETA(DisplayName = "Android"),

	/** 通用平台（跨平台共享） */
	Universal UMETA(DisplayName = "通用")
};

/**
 * 输入映射上下文配置
 * 包含平台特定的 InputMappingContext 引用
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAInputMappingContextConfig
{
	GENERATED_BODY()

	/** 平台类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mapping Context", meta = (DisplayName = "平台类型"))
	EDBAInputPlatform Platform = EDBAInputPlatform::Universal;

	/** 输入映射上下文资源 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mapping Context", meta = (DisplayName = "映射上下文"))
	TSoftObjectPtr<UInputMappingContext> MappingContext;

	/** 映射优先级（数值越大优先级越高） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mapping Context", meta = (DisplayName = "优先级"))
	int32 Priority = 0;
};
