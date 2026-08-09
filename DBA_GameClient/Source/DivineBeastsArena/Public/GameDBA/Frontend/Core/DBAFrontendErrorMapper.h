// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameDBA/Frontend/Core/DBAFrontendContracts.h"
#include "DBAFrontendErrorMapper.generated.h"

/** 将 HTTP 与后端业务代码转换为 UI 可消费的结构化错误。 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAFrontendErrorMapper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DBA|Frontend|Error")
	static FDBAApiError FromHttpStatus(int32 HttpStatusCode, FName BackendErrorCode);

	/** 将角色创建服务的稳定业务错误码转换为中文 UX 文案；Widget 不解析服务端 message。 */
	UFUNCTION(BlueprintPure, Category = "DBA|Frontend|Error")
	static FDBAApiError FromCharacterCreateErrorCode(FName BackendErrorCode, int32 HttpStatusCode = 422);

	/** 仅用于尚未迁移的字符串错误出口；新业务必须传递后端 ErrorCode 与 HTTP 状态。 */
	static FDBAApiError FromLegacyMessage(const FString& LegacyMessage);
};
