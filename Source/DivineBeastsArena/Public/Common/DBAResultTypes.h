// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/DBAErrorCodes.h"
#include "DBAResultTypes.generated.h"

/**
 * 操作结果结构体
 * 用于统一异步操作、RPC 调用、数据加载的返回值
 * 包含成功标志、错误码、错误消息、可选数据
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAOperationResult
{
	GENERATED_BODY()

	/** 操作是否成功 */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bSuccess = false;

	/** 错误码，成功时为 None */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	EDBAErrorCode ErrorCode = EDBAErrorCode::None;

	/** 错误消息，用于日志记录和调试，不直接显示给玩家 */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString ErrorMessage;

	/** 可选的附加数据，用于传递额外信息 */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString AdditionalData;

	FDBAOperationResult() = default;

	/**
	 * 创建成功结果
	 * @param InAdditionalData 可选的附加数据
	 * @return 成功结果
	 */
	static FDBAOperationResult Success(const FString& InAdditionalData = FString())
	{
		FDBAOperationResult Result;
		Result.bSuccess = true;
		Result.ErrorCode = EDBAErrorCode::None;
		Result.AdditionalData = InAdditionalData;
		return Result;
	}

	/**
	 * 创建失败结果
	 * @param InErrorCode 错误码
	 * @param InErrorMessage 错误消息
	 * @return 失败结果
	 */
	static FDBAOperationResult Failure(EDBAErrorCode InErrorCode, const FString& InErrorMessage = FString())
	{
		FDBAOperationResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = InErrorCode;
		Result.ErrorMessage = InErrorMessage;
		return Result;
	}
};

/**
 * 数据加载结果结构体
 * 用于 DataTable、DataAsset、配置文件加载的返回值
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBADataLoadResult
{
	GENERATED_BODY()

	/** 加载是否成功 */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bSuccess = false;

	/** 错误码，成功时为 None */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	EDBAErrorCode ErrorCode = EDBAErrorCode::None;

	/** 加载的资源路径 */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString AssetPath;

	/** 加载耗时（毫秒） */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float LoadTimeMs = 0.0f;

	/** 错误消息 */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString ErrorMessage;

	FDBADataLoadResult() = default;

	static FDBADataLoadResult Success(const FString& InAssetPath, float InLoadTimeMs)
	{
		FDBADataLoadResult Result;
		Result.bSuccess = true;
		Result.ErrorCode = EDBAErrorCode::None;
		Result.AssetPath = InAssetPath;
		Result.LoadTimeMs = InLoadTimeMs;
		return Result;
	}

	static FDBADataLoadResult Failure(EDBAErrorCode InErrorCode, const FString& InAssetPath, const FString& InErrorMessage)
	{
		FDBADataLoadResult Result;
		Result.bSuccess = false;
		Result.ErrorCode = InErrorCode;
		Result.AssetPath = InAssetPath;
		Result.ErrorMessage = InErrorMessage;
		return Result;
	}
};